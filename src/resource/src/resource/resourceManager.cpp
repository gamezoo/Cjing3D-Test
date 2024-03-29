#include "resourceManager.h"
#include "converter.h"
#include "core\helper\debug.h"
#include "core\filesystem\filesystem_physfs.h"
#include "core\container\hashMap.h"
#include "core\concurrency\taskJob.h"
#include "core\helper\timer.h"
#include "core\container\mpmc_bounded_queue.h"
#include "core\plugin\pluginManager.h"
#include "core\serialization\jsonArchive.h"
#include "core\platform\platform.h"
#include "core\string\stringUtils.h"

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

		// registered infos
		FactoryTable mResourceFactoires;
		ResourceTypeTable mResourceTypeTable;
		HashMap<U32, ResourceType> mRegisteredExt;

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

		
		volatile I32 mPendingResJobs = 0;
		volatile I32 mConversionJobs = 0;

		// load hook
		LoadHook* mLoadHook = nullptr;

		bool mIsInitialized = false;

	public:
		ResourceManagerImpl(BaseFileSystem* filesystem);
		~ResourceManagerImpl();

		ResourceFactory* GetFactory(ResourceType type);
		void AcquireResource(Resource* resource);
		bool ReleaseResource(Resource* resource);
		Resource* AcquireResource(const Path& path, ResourceType type);
		void RecordResource(const Path& path, ResourceType type, Resource& resource);
		void ProcessReleasedResources();
		DynamicArray<String> LoadResSources(const char* src);
		Path GetResourceConvertedPath(const Path& inPath);
	};
	ResourceManagerImpl* mImpl = nullptr;

	ResourceManagerImpl::ResourceManagerImpl(BaseFileSystem* filesystem) :
		mReadTaskQueue(MAX_READ_TASKS),
		mWriteTaskQueue(MAX_WRITE_TASKS),
		mWriteTaskSem(0, MAX_WRITE_TASKS, "ResWriteSem"),
		mReadTaskSem(0,  MAX_READ_TASKS, "ResReadSem"),
		mWriteThread(WriteIOTaskFunc, this, 65536, "WriteIOThread"),
		mReadThread(ReadIOTaskFunc, this, 65536, "ReadIOThread"),
		mFilesystem(filesystem)
	{
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
		if (resource->SubRefCount() == 0)
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

		// keep one ref count by the resTable
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
			resource->OnUnloaded();

			ResourceFactory* factory = mImpl->GetFactory(resource->GetType());
			if (factory != nullptr) {
				factory->DestroyResource(resource);
			}
		}
	}

	DynamicArray<String> ResourceManagerImpl::LoadResSources(const char* src)
	{
		DynamicArray<String> ret;
		MaxPathString metaPath(src);
		metaPath.append(".metadata");
		if (mFilesystem->IsFileExists(metaPath.c_str()))
		{
			JsonArchive archive(metaPath.c_str(), ArchiveMode::ArchiveMode_Read, mFilesystem);
			archive.Read("$internal", [&ret](JsonArchive& archive) {
				archive.Read("sources", ret);
			});
		}
		return ret;
	}

	Path ResourceManagerImpl::GetResourceConvertedPath(const Path& inPath)
	{
		// convertedPath format: inPath.converted

		MaxPathString dirPath, fileName, ext;
		if (!inPath.SplitPath(dirPath.data(), dirPath.size(), fileName.data(), fileName.size(), ext.data(), ext.size()))
		{
			Logger::Warning("Invalid path:%s", inPath.c_str());
			return Path();
		}

		MaxPathString convertedPath, convertedFileName;
		sprintf_s(convertedPath.data(), convertedPath.size(), COMPILED_PATH_NAME);
		sprintf_s(convertedFileName.data(), convertedFileName.size(), "%s.%s.converted", fileName.data(), ext.data());

		// converted full path
		Path convertedfullPath(convertedPath.c_str());
		convertedfullPath.AppendPath(Path(dirPath));
		convertedfullPath.AppendPath(Path(convertedFileName));

		return convertedfullPath;
	}

	AsyncResult FileIOTask::DoRead()
	{
		// pending -> running
		if (mHandle != nullptr)
		{
			auto oldResult = (AsyncResult)Concurrency::AtomicExchange((volatile I32*)&mHandle->mResult, (I32)AsyncResult::RUNNING);
			Debug::CheckAssertion(oldResult == AsyncResult::PENDING);
		}

		// 每次最大读取MAX_READ_SIZE
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

		// 每次最大写入MAX_WRITE_SIZE
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
		ResourceLoadJob(ResourceFactory& factory, Resource& resource, const char* name, const char* path, const char* originalPath);
		virtual ~ResourceLoadJob();

		void OnWork(I32 param)override;
		void OnCompleted()override;

	private:
		ResourceFactory& mFactory;
		Resource& mResource;
		String mName;
		// target path, maybe is ConvertedPath
		String mPath;
		// original path
		String mOriginalPath;
	};

	ResourceLoadJob::ResourceLoadJob(ResourceFactory& factory, Resource& resource, const char* name, const char*path, const char* originalPath) :
		JobSystem::TaskJob("ResourceLoadJob"),
		mFactory(factory),
		mResource(resource),
		mName(name),
		mPath(path),
		mOriginalPath(path)
	{
		resource.SetDesiredState(Resource::ResState::LOADED); // desired loaded
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
			Logger::Warning("Failed to load resource \"%s\"", mPath);
			mResource.OnLoaded(false);
			return;
		}

		bool success = mFactory.LoadResourceFromFile(&mResource, mName.c_str(), buffer.size(), (const U8*)buffer.data());
		if (success && !mResource.IsLoaded())
		{
			// try to load original path
			auto sources = mImpl->LoadResSources(mOriginalPath);
			if (!sources.empty()) {
				mResource.SetSourceFiles(sources);
			}
			mResource.SetCompiledSize(buffer.size());
			mResource.OnLoaded(true);
		}
		if (!success)
		{
			Logger::Warning("Failed to load resource \"%s\"", mPath);
			mResource.OnLoaded(false);
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
	void Initialize(BaseFileSystem* filesystem)
	{
		Debug::CheckAssertion(JobSystem::IsInitialized());
		Debug::CheckAssertion(PluginManager::IsInitialized());
		Debug::CheckAssertion(mImpl == nullptr);
		mImpl = CJING_NEW(ResourceManagerImpl)(filesystem);
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
		factory->RegisterExtensions();
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
			Logger::Warning("Empty path:%s", inPath.c_str());
			return nullptr;
		}

		MaxPathString dirPath, fileName, ext;
		if (!inPath.SplitPath(dirPath.data(), dirPath.size(), fileName.data(), fileName.size(), ext.data(), ext.size()))
		{
			Logger::Warning("Invalid path:%s", inPath.c_str());
			return nullptr;
		}

		ResourceFactory* factory = mImpl->GetFactory(type);
		if (factory == nullptr)
		{
			Logger::Warning("Invalid res type:%d", type.Type());
			return nullptr;
		}

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
		}

		if (ret->IsNeedLoad())
		{
			// onBeforeLoad
			LoadHook::HookResult hookRet = LoadHook::HookResult::IMMEDIATE;
			if (mImpl->mLoadHook != nullptr) {
				hookRet = mImpl->mLoadHook->OnBeforeLoad(ret);
			}

			if (hookRet == LoadHook::HookResult::DEFERRED)
			{
				ret->SetDesiredState(Resource::ResState::LOADED);
				mImpl->AcquireResource(ret);	// for hook
				Concurrency::AtomicIncrement(&mImpl->mPendingResJobs); // for hook
				return ret;
			}

			// converted full path
			Path convertedfullPath = mImpl->GetResourceConvertedPath(inPath);
			ret->SetConvertedPath(convertedfullPath);

			if (mImpl->mFilesystem->IsFileExists(convertedfullPath.c_str()))
			{
				auto* loadJob = CJING_NEW(ResourceLoadJob)(*factory, *ret, inPath.c_str(), convertedfullPath.c_str(), inPath.c_str());
				if (isImmediate) {
					loadJob->RunTaskImmediate(0);
				}
				else {
					loadJob->RunTask(0, JobSystem::Priority::LOW);
				}
				return ret;
			}
			else
			{
				mImpl->ReleaseResource(ret);
				Logger::Warning("The convert_output of \'%s\' is not exists", inPath.c_str());
				return nullptr;
			}
		}
		return ret;                                                                                        
	}

	Path GetResourceConvertedPath(const Path& inPath)
	{
		return mImpl->GetResourceConvertedPath(inPath);
	}

	void ReloadResource(Resource* resource)
	{
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

	void ProcessReleasedResources()
	{
		Debug::CheckAssertion(IsInitialized());
		return mImpl->ProcessReleasedResources();
	}

	void WaitForResource(Resource* resource)
	{
		Debug::CheckAssertion(IsInitialized());
		if (resource == nullptr) {
			return;
		}

#ifdef _DEBUG
		F64 maxWaitTime = 100.0f;
#else
		F64 maxWaitTime = 10.0f;
#endif
		F64 startTime = Timer::GetAbsoluteTime();
		while(!resource->IsLoaded() && !resource->IsFaild())
		{
			if (mImpl->mLoadHook != nullptr) {
				mImpl->mLoadHook->OnWait();
			}

			JobSystem::YieldCPU();
			if (Timer::GetAbsoluteTime() - startTime > maxWaitTime)
			{
#ifdef _DEBUG
				Logger::Warning("Resource load time out, try loading again");
				maxWaitTime *= 2;
#else
				Logger::Error("Resource load time out");
				break;
#endif
			}
		}
	}

	void WaitForResources(Span<Resource*> resources)
	{
		Debug::CheckAssertion(IsInitialized());
		if (resources.length() == 0) {
			return;
		}

#ifdef _DEBUG
		F64 maxWaitTime = 100.0f;
#else
		F64 maxWaitTime = 10.0f;
#endif
		F64 startTime = Timer::GetAbsoluteTime();
		bool isAllLoaded = true;
		for (Resource* res : resources) 
		{
			if (!res->IsLoaded() && !res->IsFaild()) 
			{
				isAllLoaded = false;
				break;
			}
		}

		while (!isAllLoaded)
		{
			if (mImpl->mLoadHook != nullptr) {
				mImpl->mLoadHook->OnWait();
			}

			JobSystem::YieldCPU();

			if (Timer::GetAbsoluteTime() - startTime > maxWaitTime)
			{
#ifdef _DEBUG
				Logger::Warning("Resource load time out, try loading again");
				maxWaitTime *= 2;
#else
				Logger::Error("Resource load time out");
				break;
#endif
			}

			isAllLoaded = true;
			for (Resource* res : resources)
			{
				if (!res->IsLoaded() && !res->IsFaild())
				{
					isAllLoaded = false;
					break;
				}
			}
		}
	}

	void WaitAll()
	{
		F64 maxWaitTime = 100000.0f;
		F64 startTime = Timer::GetAbsoluteTime();

		while (mImpl->mPendingResJobs > 0) 
		{
			if (mImpl->mLoadHook != nullptr) {
				mImpl->mLoadHook->OnWait();
			}

			JobSystem::YieldCPU();

			if (Timer::GetAbsoluteTime() - startTime > maxWaitTime)
			{
#ifdef _DEBUG
				Logger::Warning("Resource load time out, try loading again");
				maxWaitTime *= 2;
#else
				Logger::Error("Resource load time out");
				break;
#endif
			}
		}
	}

	void RegisterExtension(const char* ext, ResourceType type)
	{
		mImpl->mRegisteredExt.insert(StringUtils::StringToHash(ext), type);
	}

	ResourceType GetResourceType(const char* path)
	{
		// need register resourceType by extension (RegisterExtension)
		MaxPathString ext;
		Path::GetPathExtension(Span(path, StringLength(path)), ext.toSpan());
		auto it = mImpl->mRegisteredExt.find(StringUtils::StringToHash(ext.c_str()));
		if (it != nullptr) {
			return *it;
		}
		return ResourceType::INVALID_TYPE;
	}

	AsyncResult ReadFileData(const char* path, char*& buffer, size_t& size, AsyncHandle* asyncHanlde)
	{
		Debug::CheckAssertion(IsInitialized());

		if (!mImpl->mFilesystem->IsFileExists(path))
		{
			Logger::Warning("Empty path:%s", path);
			return AsyncResult::FAILURE;
		}

		File* file = CJING_NEW(File);
		if (!mImpl->mFilesystem->OpenFile(path, *file, FileFlags::DEFAULT_READ)) 
		{
			Logger::Warning("File open failed, path:%s", path);
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
			Logger::Warning("File write failed, path:%s", path);
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

	void SetCurrentLoadHook(LoadHook* loadHook)
	{
		Debug::CheckAssertion(IsInitialized());
		mImpl->mLoadHook = loadHook;
	}

	void LoadHook::ContinueLoad(Resource* res, bool isImmediate)
	{
		if (res->IsFaild()) {
			return;
		}
		Debug::CheckAssertion(res->IsEmpty());
		Concurrency::AtomicDecrement(&mImpl->mPendingResJobs); // release from hook
		mImpl->ReleaseResource(res);		// release from hook
		res->SetDesiredState(Resource::ResState::EMPTY);

		// load resource
		ResourceFactory* factory = mImpl->GetFactory(res->GetType());
		if (factory == nullptr) {
			Logger::Warning("Invalid res type:%d", res->GetType().Type());
			return;
		}

		const Path& inPath = res->GetPath();

		// converted full path
		Path convertedfullPath = mImpl->GetResourceConvertedPath(inPath);
		res->SetConvertedPath(convertedfullPath);

		if (mImpl->mFilesystem->IsFileExists(convertedfullPath.c_str()))
		{
			auto* loadJob = CJING_NEW(ResourceLoadJob)(*factory, *res, inPath.c_str(), convertedfullPath.c_str(), inPath.c_str());
			if (isImmediate) {
				loadJob->RunTaskImmediate(0);
			}
			else {
				loadJob->RunTask(0, JobSystem::Priority::LOW);
			}
		}
		else
		{
			mImpl->ReleaseResource(res);
			Logger::Warning("The convert_output of \'%s\' is not exists", inPath.c_str());
			return;
		}
	}
}
}