#include "resourceManager.h"
#include "core\helper\debug.h"
#include "core\filesystem\filesystem_physfs.h"
#include "core\container\hashMap.h"
#include "core\jobsystem\taskJob.h"
#include "core\helper\timer.h"
#include "core\container\mpmc_bounded_queue.h"

namespace Cjing3D
{
namespace ResourceManager
{
	//////////////////////////////////////////////////////////////////////////
	// File IO
	//////////////////////////////////////////////////////////////////////////
	struct FileIOTask
	{
		static const size_t MAX_READ_SIZE = 8 * 1024 * 1024;
		static const size_t MAX_WRITE_SIZE = 8 * 1024 * 1024;

		File*  mFile;
		size_t mOffset;
		size_t mSize;
		void*  mAddr;
		AsyncHandle* mHandle;

		AsyncResult DoRead();
		AsyncResult DoWrite();
	};

	static int ReadIOTaskFunc(void* data);
	static int WriteIOTaskFunc(void* data);

	//////////////////////////////////////////////////////////////////////////
	// Impl
	//////////////////////////////////////////////////////////////////////////
	class ResourceManagerImpl
	{
	public:
		using FactoryTable = HashMap<U32, ResourceFactory*>;
		using ResourceTable = HashMap<U32, Resource*>;
		using ResourceTypeTable = HashMap<U32, ResourceTable>;

		static const I32 MAX_READ_TASKS = 128;
		static const I32 MAX_WRITE_TASKS = 128;

		FactoryTable mResourceFactoires;
		ResourceTypeTable mResourceTypeTable;
		DynamicArray<Resource*> mReleasedResources;

		MaxPathString mRootPath;
		Concurrency::RWLock mRWLock;
		BaseFileSystem* mFilesystem = nullptr;

		// resource file io
		Concurrency::Semaphore mWriteTaskSem;
		Concurrency::Semaphore mReadTaskSem;
		MPMCBoundedQueue<FileIOTask> mWriteTaskQueue;
		MPMCBoundedQueue<FileIOTask> mReadTaskQueue;
		Concurrency::Thread mWriteThread;
		Concurrency::Thread mReadThread;
		bool mIOExiting = false;

		volatile I32 mPendingResJobs = 0;
		bool mIsInitialized = false;

	public:
		ResourceManagerImpl(const char* rootPath);
		~ResourceManagerImpl();

		ResourceFactory* GetFactory(ResourceType type);
		void AcquireResource(Resource* resource);
		bool ReleaseResource(Resource* resource);
		Resource* AcquireResource(const Path& path, ResourceType type);
		void RecordResource(const Path& path, ResourceType type, Resource& resource);
		void ProcessReleasedResources();
	};
	ResourceManagerImpl* mImpl = nullptr;

	ResourceManagerImpl::ResourceManagerImpl(const char* rootPath) :
		mReadTaskQueue(MAX_READ_TASKS),
		mWriteTaskQueue(MAX_WRITE_TASKS),
		mWriteTaskSem(0, MAX_WRITE_TASKS, "ResWriteSem"),
		mReadTaskSem(0,  MAX_READ_TASKS, "ResReadSem"),
		mWriteThread(WriteIOTaskFunc, this, 65536, "WriteIOThread"),
		mReadThread(ReadIOTaskFunc, this, 65536, "ReadIOThread")
	{
		mFilesystem = CJING_NEW(FileSystemPhysfs)(rootPath);
		mIsInitialized = true;
	}

	ResourceManagerImpl::~ResourceManagerImpl()
	{
		mIsInitialized = false;

		while (mPendingResJobs > 0) {
			JobSystem::YieldCPU();
		}

		// process released resource
		ProcessReleasedResources();

		// exit io thread
		mIOExiting = true;
		mReadTaskSem.Signal(1);
		mReadThread.Join();

		mWriteTaskSem.Signal(1);
		mWriteThread.Join();

		CJING_SAFE_DELETE(mFilesystem);
	}

	ResourceFactory* ResourceManagerImpl::GetFactory(ResourceType type)
	{
		auto it = mResourceFactoires.find(type.Type());
		return it != nullptr ? *it : nullptr;
	}

	void ResourceManagerImpl::AcquireResource(Resource* resource)
	{
		resource->AddRefCount();
	}

	bool ResourceManagerImpl::ReleaseResource(Resource* resource)
	{
		if (Concurrency::AtomicDecrement(&resource->mRefCount) == 0)
		{
			Concurrency::ScopedWriteLock lock(mRWLock);
			ResourceTable* resTable = mResourceTypeTable.find(resource->GetType().Type());
			if (resTable == nullptr) {
				return false;
			}

			U32 key = resource->GetPath().GetHash();
			auto it = resTable->find(key);
			if (it != nullptr) {
				resTable->erase(key);
			}

			mReleasedResources.push(*it);	
			return true;
		}
		return false;
	}

	Resource* ResourceManagerImpl::AcquireResource(const Path& path, ResourceType type)
	{
		Concurrency::ScopedReadLock lock(mRWLock);
		ResourceTable* resTable = mResourceTypeTable.find(type.Type());
		if (resTable == nullptr) {
			return nullptr;
		}

		auto it = resTable->find(path.GetHash());
		if (it == nullptr) {
			return nullptr;
		}

		Resource* ret = *it;
		ret->AddRefCount();
		return ret;
	}

	void ResourceManagerImpl::RecordResource(const Path& path, ResourceType type, Resource& resource)
	{
		Concurrency::ScopedWriteLock lock(mRWLock);
		ResourceTable* resTable = mResourceTypeTable.find(type.Type());
		if (resTable == nullptr) {
			resTable = mResourceTypeTable.insert(type.Type(), ResourceTable());
		}
		resTable->insert(path.GetHash(), &resource);
		resource.AddRefCount();
	}

	void ResourceManagerImpl::ProcessReleasedResources()
	{
		DynamicArray<Resource*> releasedResources;
		{
			Concurrency::ScopedWriteLock lock(mRWLock);
			releasedResources = mReleasedResources;
			mReleasedResources.clear();
		}

		for (auto resource : releasedResources)
		{
			Debug::CheckAssertion(resource->IsLoaded());

			ResourceFactory* factory = mImpl->GetFactory(resource->GetType());
			if (factory != nullptr) {
				factory->DestroyResource(resource);
			}
		}
	}

	AsyncResult FileIOTask::DoRead()
	{
		// pending -> running
		if (mHandle != nullptr)
		{
			auto oldResult = (AsyncResult)Concurrency::AtomicExchange((volatile I32*)&mHandle->mResult, (I32)AsyncResult::RUNNING);
			Debug::CheckAssertion(oldResult == AsyncResult::PENDING);
		}

		char* dest = (char*)mAddr;
		size_t remainingSize = mSize;
		while (remainingSize > 0)
		{
			size_t readBytes = std::min(MAX_READ_SIZE, remainingSize);
			if (!mFile->Read(dest, readBytes)) {
				break;
			}

			dest += readBytes;
			remainingSize -= readBytes;

			if (mHandle != nullptr) {
				Concurrency::AtomicAddRelease((volatile I32*)&mHandle->mRemaining, -((I32)readBytes));
			}
		}

		AsyncResult ret = AsyncResult::FAILURE;
		if (remainingSize <= 0) {
			ret = AsyncResult::SUCCESS;
		}

		// running -> ret
		if (mHandle != nullptr) {
			Concurrency::AtomicExchange((volatile I32*)&mHandle->mResult, (I32)ret);
		}

		// TODO:
		CJING_SAFE_DELETE(mFile);
		return ret;
	}

	AsyncResult FileIOTask::DoWrite()
	{
		// pending -> running
		if (mHandle != nullptr)
		{
			auto oldResult = (AsyncResult)Concurrency::AtomicExchange((volatile I32*)&mHandle->mResult, (I32)AsyncResult::RUNNING);
			Debug::CheckAssertion(oldResult == AsyncResult::PENDING);
		}

		char* dest = (char*)mAddr;
		size_t remainingSize = mSize;
		while (remainingSize > 0)
		{
			size_t writeBytes = std::min(MAX_WRITE_SIZE, remainingSize);
			if (!mFile->Write(dest, writeBytes)) {
				break;
			}

			dest += writeBytes;
			remainingSize -= writeBytes;

			if (mHandle != nullptr) {
				Concurrency::AtomicAddRelease((volatile I32*)&mHandle->mRemaining, -((I32)writeBytes));
			}
		}

		AsyncResult ret = AsyncResult::FAILURE;
		if (remainingSize <= 0) {
			ret = AsyncResult::SUCCESS;
		}

		// running -> ret
		if (mHandle != nullptr) {
			Concurrency::AtomicExchange((volatile I32*)&mHandle->mResult, (I32)ret);
		}

		// TODO:
		CJING_SAFE_DELETE(mFile);
		return ret;
	}

	static int ReadIOTaskFunc(void* data)
	{
		auto* impl = reinterpret_cast<ResourceManagerImpl*>(data);
		FileIOTask task;
		while (true)
		{
			impl->mReadTaskSem.Wait();
			if (impl->mReadTaskQueue.Dequeue(task)) {
				task.DoRead();
			}

			if (mImpl->mIOExiting) {
				break;
			}
		}
		return 0;
	}

	static int WriteIOTaskFunc(void* data)
	{
		auto* impl = reinterpret_cast<ResourceManagerImpl*>(data);
		FileIOTask task;
		while (true)
		{
			impl->mWriteTaskSem.Wait();
			if (impl->mWriteTaskQueue.Dequeue(task)) {
				task.DoWrite();
			}

			if (mImpl->mIOExiting) {
				break;
			}
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	// Task job
	//////////////////////////////////////////////////////////////////////////
	class ResourceLoadJob : public JobSystem::TaskJob
	{
	public:
		ResourceLoadJob(ResourceFactory& factory, Resource& resource, const char*name, const char* path);
		virtual ~ResourceLoadJob();

		void OnWork(I32 param)override;
		void OnCompleted()override;

	private:
		ResourceFactory& mFactory;
		Resource& mResource;
		String mName;
		String mPath;
	};

	ResourceLoadJob::ResourceLoadJob(ResourceFactory& factory, Resource& resource, const char* name, const char*path) :
		JobSystem::TaskJob("ResourceLoadJob"),
		mFactory(factory),
		mResource(resource),
		mName(name),
		mPath(path)
	{
		mImpl->AcquireResource(&resource);	// add ref
		Concurrency::AtomicIncrement(&mImpl->mPendingResJobs);
	}

	ResourceLoadJob::~ResourceLoadJob()
	{
		Concurrency::AtomicDecrement(&mImpl->mPendingResJobs);
	}

	void ResourceLoadJob::OnWork(I32 param)
	{
		DynamicArray<char> buffer;
		if (!mImpl->mFilesystem->ReadFile(mPath, buffer)) 
		{
			Debug::CheckAssertion("Failed to load resource \"%s\"", mPath);
			return;
		}

		File file(buffer.data(), buffer.size(), FileFlags::DEFAULT_READ);
		bool success = mFactory.LoadResource(&mResource, mName.c_str(), file);
		if (!success)
		{
			Debug::CheckAssertion("Failed to load resource \"%s\"", mPath);
			return;
		}
		Concurrency::AtomicIncrement(&mResource.mLoaded);
	}

	void ResourceLoadJob::OnCompleted()
	{
		mImpl->ReleaseResource(&mResource);
		CJING_DELETE(this);
	}

	//////////////////////////////////////////////////////////////////////////
	// Manager
	//////////////////////////////////////////////////////////////////////////
	void Initialize(const char* rootPath)
	{
		Debug::CheckAssertion(mImpl == nullptr);
		mImpl = CJING_NEW(ResourceManagerImpl)(rootPath);
	}

	void Uninitialize()
	{
		Debug::CheckAssertion(mImpl != nullptr);
		CJING_SAFE_DELETE(mImpl);
	}

	bool IsInitialized()
	{
		return mImpl != nullptr;
	}

	void RegisterFactory(ResourceType type, ResourceFactory* factory)
	{
		if (!IsInitialized()) {
			return;
		}
		mImpl->mResourceFactoires.insert(type.Type(), factory);
	}

	void UnregisterFactory(ResourceType type)
	{
		if (!IsInitialized()) {
			return;
		}
		mImpl->mResourceFactoires.erase(type.Type());
	}

	Resource* LoadResource(ResourceType type, const Path& inPath, bool isImmediate)
	{
		Debug::CheckAssertion(IsInitialized());
		Debug::CheckAssertion(!inPath.IsEmpty());

		if (!mImpl->mFilesystem->IsFileExists(inPath.c_str())) 
		{
			Debug::Warning("Empty path:%s", inPath.c_str());
			return nullptr;
		}

		MaxPathString dirPath, fileName, ext;
		if (!inPath.SplitPath(dirPath.data(), dirPath.size(), fileName.data(), fileName.size(), ext.data(), ext.size()))
		{
			Debug::Warning("Invalid path:%s", inPath.c_str());
			return nullptr;
		}

		ResourceFactory* factory = mImpl->GetFactory(type);
		if (factory == nullptr)
		{
			Debug::Warning("Invalid res type:%d", type.Type());
			return nullptr;
		}

		if (!factory->IsNeedConvert())
		{
			// acquire resource
			Resource* ret = mImpl->AcquireResource(inPath, type);
			if (ret == nullptr)
			{
				ret = factory->CreateResource();
				if (ret == nullptr) {
					return nullptr;
				}
				mImpl->RecordResource(inPath, type, *ret);

				ret->SetPath(inPath);

				auto* loadJob = CJING_NEW(ResourceLoadJob)(*factory, *ret, fileName.c_str(), inPath.c_str());
				if (isImmediate) {
					loadJob->RunTaskImmediate(0);
				}
				else {
					loadJob->RunTask(0, JobSystem::Priority::LOW);
				}
			}
			return ret;
		}
		else
		{
			// set converted path
			MaxPathString convertedPath, convertedFileName;
			sprintf_s(convertedFileName.data(), convertedFileName.size(), "%s.%s.converted", fileName.data(), ext.data());
			sprintf_s(convertedPath.data(), convertedPath.size(), "%s.converter_output", mImpl->mRootPath.data());

			// acquire resource
			Resource* ret = mImpl->AcquireResource(Path(convertedPath), type);
			if (ret == nullptr)
			{
				ret = factory->CreateResource();
				if (ret == nullptr) {
					return nullptr;
				}
				// add ref
				mImpl->RecordResource(Path(convertedPath), type, *ret);

				ret->SetPath(Path(convertedPath));

				// todo

			}
			return ret;
		}
	}
	void AcquireResource(Resource* resource)
	{
		Debug::CheckAssertion(IsInitialized());
		if (resource != nullptr) {
			mImpl->AcquireResource(resource);
		}
	}

	bool ReleaseResource(Resource** resource)
	{
		Debug::CheckAssertion(IsInitialized());
		WaitForResource(*resource);
		if (mImpl->ReleaseResource(*resource)) {
			mImpl->ProcessReleasedResources();
		}

		*resource = nullptr;
		return true;
	}
	bool IsResourceLoaded(Resource* resource)
	{
		Debug::CheckAssertion(IsInitialized());
		return resource->IsLoaded();
	}

	void WaitForResource(Resource* resource)
	{
		Debug::CheckAssertion(IsInitialized());
#ifdef _DEBUG
		F64 maxWaitTime = 100.0f;
#else
		F64 maxWaitTime = 10.0f;
#endif
		F64 startTime = Timer::GetAbsoluteTime();
		while(!IsResourceLoaded(resource))
		{
			JobSystem::YieldCPU();
			if (Timer::GetAbsoluteTime() - startTime > maxWaitTime)
			{
				Debug::Warning("Resource load time out");
#ifdef _DEBUG
				maxWaitTime *= 2;
#else
				break;
#endif
			}
		}
	}

	AsyncResult ReadFileData(const char* path, char*& buffer, size_t& size, AsyncHandle* asyncHanlde)
	{
		Debug::CheckAssertion(IsInitialized());

		if (!mImpl->mFilesystem->IsFileExists(path))
		{
			Debug::Warning("Empty path:%s", path);
			return AsyncResult::FAILURE;
		}

		File* file = CJING_NEW(File);
		if (!mImpl->mFilesystem->OpenFile(path, *file, FileFlags::DEFAULT_READ)) 
		{
			Debug::Warning("File open failed, path:%s", path);
			return AsyncResult::FAILURE;
		}

		// set async flag
		if (asyncHanlde != nullptr)
		{
			auto oldResult = (AsyncResult)Concurrency::AtomicExchange((volatile I32*)&asyncHanlde->mResult, (I32)AsyncResult::PENDING);
			Debug::CheckAssertion(oldResult == AsyncResult::EMPTY);
		}

		size = file->Size();
		buffer = CJING_NEW_ARR(char, size);

		FileIOTask task;
		task.mOffset = 0;
		task.mSize = size;
		task.mHandle = asyncHanlde;
		task.mAddr = buffer;
		task.mFile = file;

		if (asyncHanlde == nullptr)
		{
			return task.DoRead();
		}
		else
		{
			Concurrency::AtomicAddAcquire(&asyncHanlde->mRemaining, (I64)size);
			mImpl->mReadTaskQueue.Enqueue(task);
			mImpl->mReadTaskSem.Signal(1);
			return AsyncResult::PENDING;
		}
	}

	AsyncResult WriteFileData(const char* path, void* buffer, size_t size, AsyncHandle* asyncHanlde)
	{
		Debug::CheckAssertion(IsInitialized());

		File* file = CJING_NEW(File);
		if (!mImpl->mFilesystem->OpenFile(path, *file, FileFlags::DEFAULT_WRITE))
		{
			Debug::Warning("File write failed, path:%s", path);
			return AsyncResult::FAILURE;
		}

		// set async flag
		if (asyncHanlde != nullptr)
		{
			auto oldResult = (AsyncResult)Concurrency::AtomicExchange((volatile I32*)&asyncHanlde->mResult, (I32)AsyncResult::PENDING);
			Debug::CheckAssertion(oldResult == AsyncResult::EMPTY);
		}

		FileIOTask task;
		task.mOffset = 0;
		task.mSize = size;
		task.mHandle = asyncHanlde;
		task.mAddr = buffer;
		task.mFile = file;

		if (asyncHanlde == nullptr)
		{
			return task.DoWrite();
		}
		else
		{
			Concurrency::AtomicAddAcquire(&asyncHanlde->mRemaining, (I64)size);
			mImpl->mWriteTaskQueue.Enqueue(task);
			mImpl->mWriteTaskSem.Signal(1);
			return AsyncResult::PENDING;
		}
	}
}
}