#include "resourceManager.h"
#include "converter.h"
#include "core\helper\debug.h"
#include "core\filesystem\filesystem_physfs.h"
#include "core\container\hashMap.h"
#include "core\jobsystem\taskJob.h"
#include "core\helper\timer.h"
#include "core\container\mpmc_bounded_queue.h"
#include "core\plugin\pluginManager.h"

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

		// converter plugins
		DynamicArray<ResConverterPlugin*> mConverterPlugins;

		volatile I32 mPendingResJobs = 0;
		volatile I32 mConversionJobs = 0;
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
		// acquire all converter plugins
		DynamicArray<Plugin*> plugins;
		PluginManager::GetPlugins<ResConverterPlugin>(plugins);
		for (auto plugin : plugins) {
			mConverterPlugins.push(reinterpret_cast<ResConverterPlugin*>(plugin));
		}

		// setup filesystem
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
			Debug::CheckAssertion(resource->IsLoaded() || resource->IsFaild());

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

	// resource load job
	class ResourceLoadJob : public JobSystem::TaskJob
	{
	public:
		ResourceLoadJob(ResourceFactory& factory, Resource& resource, const char* name, const char* path);
		virtual ~ResourceLoadJob();

		void OnWork(I32 param)override;
		void OnCompleted()override;

	private:
		ResourceFactory& mFactory;
		Resource& mResource;
		String mName;
		String mPath;
	};

	// Resource convert job
	class ResourceConvertJob : public JobSystem::TaskJob
	{
	public:
		ResourceConvertJob(Resource& resource, const ResourceType& resType, const char* srcPath, const char* convertedPath);
		virtual ~ResourceConvertJob();

		void SetLoadJob(ResourceLoadJob* job) { mResLoadJob = job; }
		void SetIsImmediate(bool isImmediate) { mIsImmediate = isImmediate; }
		void OnWork(I32 param)override;
		void OnCompleted()override;

	private:
		Resource& mResource;
		ResourceType mResType;
		const char* mSrcPath;
		const char* mConvertedPath;
		ResourceLoadJob* mResLoadJob;
		bool mConversionRet;

		bool mIsImmediate = false;
		I32 mCurrentConvertTime = 0;
		static const I32 MaxConvertTimes = 3;
	};

	ResourceConvertJob::ResourceConvertJob(Resource& resource, const ResourceType& resType, const char* srcPath, const char* convertedPath) :
		JobSystem::TaskJob("ResourceConvertJob"),
		mResource(resource),
		mResType(resType),
		mSrcPath(srcPath),
		mConvertedPath(convertedPath),
		mResLoadJob(nullptr),
		mConversionRet(false)
	{
		Concurrency::AtomicIncrement(&mImpl->mPendingResJobs);
		Debug::CheckAssertion(Concurrency::AtomicIncrement(&mResource.mConverting) == 1);
	}

	ResourceConvertJob::~ResourceConvertJob()
	{
		Concurrency::AtomicDecrement(&mImpl->mPendingResJobs);
		Concurrency::AtomicDecrement(&mResource.mConverting);
	}

	void ResourceConvertJob::OnWork(I32 param)
	{
		Concurrency::AtomicIncrement(&mImpl->mConversionJobs);
		for (auto plugin : mImpl->mConverterPlugins)
		{
			auto converter = plugin->CreateConverter();
			if (converter && converter->SupportsType(nullptr, mResType))
			{
				// convert src resource
				ResConverterContext context(*mImpl->mFilesystem);
				mConversionRet = context.Convert(converter, mResType, mSrcPath, mConvertedPath);
			}
			plugin->DestroyConverter(converter);

			// 成功转换后则直接返回
			if (mConversionRet) {
				break;
			}
		}
		Concurrency::AtomicDecrement(&mImpl->mConversionJobs);
	}

	void ResourceConvertJob::OnCompleted()
	{
		// if convert failed, try again
		if (!mConversionRet)
		{
			if (mCurrentConvertTime < MaxConvertTimes)
			{
				mCurrentConvertTime++;
				if (mIsImmediate) {
					return RunTaskImmediate(0);
				}
				else {
					return RunTask(0, JobSystem::Priority::LOW);
				}
			}
			else
			{
				Concurrency::AtomicIncrement(&mResource.mFailed);
			}
		}

		// if convert success, try to do loading job
		if (mResLoadJob)
		{
			if (mConversionRet)
			{
				if (mIsImmediate) {
					mResLoadJob->RunTaskImmediate(0);
				}
				else
				{
					JobSystem::JobHandle jobHandle;
					mResLoadJob->RunTask(0, JobSystem::Priority::LOW, &jobHandle);
					JobSystem::Wait(&jobHandle);
				}
			}
			else
			{
				mResLoadJob->OnCompleted();
				mResLoadJob = nullptr;
			}
		}
		CJING_DELETE(this);
	}

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
			Debug::Warning("Failed to load resource \"%s\"", mPath);
			Concurrency::AtomicIncrement(&mResource.mFailed);
			return;
		}

		File file(buffer.data(), buffer.size(), FileFlags::DEFAULT_READ);
		bool success = mFactory.LoadResource(&mResource, mName.c_str(), file);
		if (success && !mResource.IsLoaded())
		{
			Concurrency::AtomicIncrement(&mResource.mLoaded);
		}
		if (!success)
		{
			Debug::Warning("Failed to load resource \"%s\"", mPath);
			Concurrency::AtomicIncrement(&mResource.mFailed);
			return;
		}
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
		Debug::CheckAssertion(JobSystem::IsInitialized());
		Debug::CheckAssertion(PluginManager::IsInitialized());
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

				// set converted directory path
				MaxPathString convertedPath, convertedFileName;
				sprintf_s(convertedPath.data(), convertedPath.size(), "converter_output");
				sprintf_s(convertedFileName.data(), convertedFileName.size(), "%s.%s.converted", fileName.data(), ext.data());

				if (!mImpl->mFilesystem->IsDirExists(convertedPath.c_str())) {
					mImpl->mFilesystem->CreateDir(convertedPath.c_str());
				}

				// converted full path
				Path convertedfullPath(convertedPath.c_str());
				convertedfullPath.AppendPath(Path(dirPath));
				convertedfullPath.AppendPath(Path(convertedFileName));
				ret->SetConvertedPath(convertedfullPath);

				bool needConvert = true;
				if (mImpl->mFilesystem->IsFileExists(convertedfullPath.c_str()))
				{
					MaxPathString metaPath(inPath.c_str());
					metaPath.append(".metadata");

					if (mImpl->mFilesystem->IsFileExists(metaPath.c_str()))
					{
						// check res metadata last modified time
						U64 srcModTime = mImpl->mFilesystem->GetLastModTime(inPath.c_str());
						U64 metaModTime = mImpl->mFilesystem->GetLastModTime(metaPath.c_str());

						if (metaModTime >= srcModTime) {
							needConvert = false;
						}
					}
				}

				if (needConvert == true)
				{
					// load job
					auto* loadJob = CJING_NEW(ResourceLoadJob)(*factory, *ret, fileName.c_str(), convertedfullPath.c_str());
					
					// convert job
					auto* convertJob = CJING_NEW(ResourceConvertJob)(*ret, type, inPath.c_str(), convertedfullPath.c_str());
					convertJob->SetLoadJob(loadJob);
					convertJob->SetIsImmediate(isImmediate);
					
					if (isImmediate) {
						convertJob->RunTaskImmediate(0);
					}
					else {
						convertJob->RunTask(0, JobSystem::Priority::LOW);
					}
				}
				else
				{
					auto* loadJob = CJING_NEW(ResourceLoadJob)(*factory, *ret, fileName.c_str(), convertedfullPath.c_str());
					if (isImmediate) {
						loadJob->RunTaskImmediate(0);
					}
					else {
						loadJob->RunTask(0, JobSystem::Priority::LOW);
					}
				}
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
		while(!resource->IsLoaded() && !resource->IsFaild())
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