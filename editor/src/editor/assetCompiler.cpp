#include "assetCompiler.h"
#include "editor.h"
#include "filesWatcher.h"
#include "resource\resourceManager.h"
#include "core\concurrency\concurrency.h"
#include "core\container\mpmc_bounded_queue.h"
#include "core\serialization\jsonArchive.h"
#include "core\signal\connection.h"
#include "core\string\stringUtils.h"

namespace Cjing3D
{
	/// //////////////////////////////////////////////////////////////////////////////////
	/// Utils
	static const char* FilterPath[] = {
		"Cjing3D.log",
		"default_editor.ini",
		"editor.ini",
		"build_config.json"
	};
	static bool CheckReserved(const char* path)
	{
		for (int i = 0; i < ARRAYSIZE(FilterPath); i++)
		{
			if (EqualString(path, FilterPath[i])) {
				return true;
			}
		}
		return false;
	}

	static U32 GetResDirHash(const char* path)
	{
		MaxPathString dirPath;
		Path::GetPathParentPath(path, dirPath.toSpan());
		return StringUtils::StringToHash(dirPath.c_str());
	}

	/// //////////////////////////////////////////////////////////////////////////////////
	/// AssetCompilerImpl
	struct ResCompileTask
	{
		Resource* mRes = nullptr;
		Path mInPath;
	};

	class AssetCompilerImpl;
	struct AssetCompilerHook : public ResourceManager::LoadHook
	{
		AssetCompilerHook(AssetCompilerImpl& impl) : mImpl(impl) {}

		virtual HookResult OoBeforeLoad(Resource* res);
		virtual void OnWait();
		AssetCompilerImpl& mImpl;
	};

	class AssetCompilerImpl
	{
	public:
		GameEditor& mGameEditor;
		BaseFileSystem& mFileSystem;
		AssetCompilerHook mLoadHook;
		FilesWatcher* mFilesWatcher = nullptr;
		ScopedConnection mFileChangedConn;

		Concurrency::Thread mCompileThread;
		Concurrency::Semaphore mCompileSemaphore;

		Concurrency::RWLock mRWLock;
		MPMCBoundedQueue<ResCompileTask> mToCompileTasks;
		MPMCBoundedQueue<ResCompileTask> mCompiledTasks;
		volatile I32 mPendingTasks = 0;

		HashMap<U32, AssetCompiler::ResourceItem> mResources;
		Signal<void()> OnListChanged;
		DynamicArray<Path> mChangedFiles;

		bool mIsExit = false;

	public:
		AssetCompilerImpl(GameEditor& gameEditor);
		~AssetCompilerImpl();

		void SetupAssets();
		void ScanDirectory(const char* dir, U64 lastModTime);
		bool Compile(const ResCompileTask& task);
		ResourceManager::LoadHook::HookResult OnBeforeLoad(Resource* res);
		void ProcessCompiledTasks();
		void ProcessChangedFiles();
		void RecoredResources();
		void OnFileChanged(const char* path);
		void AddResource(const char* fullPath);
		void AddResource(ResourceType type, const char* path);
	};

	ResourceManager::LoadHook::HookResult AssetCompilerHook::OoBeforeLoad(Resource* res)
	{
		return mImpl.OnBeforeLoad(res);
	}

	void AssetCompilerHook::OnWait()
	{
		mImpl.ProcessCompiledTasks();
	}

	static int CompileTaskFunc(void* data)
	{
		auto* impl = reinterpret_cast<AssetCompilerImpl*>(data);
		while (!impl->mIsExit)
		{
			impl->mCompileSemaphore.Wait();

			ResCompileTask task;
			if (!impl->mToCompileTasks.Dequeue(task)) {
				continue;
			}

			if (!impl->Compile(task)) {
				Logger::Error("Failed to compile resource:%s", task.mInPath.c_str());
			}

			impl->mCompiledTasks.Enqueue(task);
		}
		return 0;
	}

	AssetCompilerImpl::AssetCompilerImpl(GameEditor& gameEditor) :
		mGameEditor(gameEditor),
		mFileSystem(*gameEditor.GetEngine()->GetFileSystem()),
		mCompileThread(CompileTaskFunc, this, 65536, "CompileThread"),
		mCompileSemaphore(0, 128, "CompileSem"),
		mCompiledTasks(128),
		mToCompileTasks(128),
		mLoadHook(*this)
	{
		if (!mFileSystem.IsDirExists(COMPILED_PATH_NAME)) {
			mFileSystem.CreateDir(COMPILED_PATH_NAME);
		}
		ResourceManager::SetCurrentLoadHook(&mLoadHook);

		// files watcher
		const char* path = mFileSystem.GetBasePath();
		mFilesWatcher = CJING_NEW(FilesWatcher)(path);
		mFileChangedConn = mFilesWatcher->GetOnFilesChanged().Connect([&](const char* path) {
			OnFileChanged(path);
		});

		Logger::Info("Asset compiler initialzied");
	}

	AssetCompilerImpl::~AssetCompilerImpl()
	{
		mIsExit = true;

		mFileChangedConn.Disconnect();
		CJING_SAFE_DELETE(mFilesWatcher);

		ResourceManager::SetCurrentLoadHook(nullptr);
		mCompileSemaphore.Signal(1);
		mCompileThread.Join();

		Logger::Info("Asset compiler uninitialized");
	}

	void AssetCompilerImpl::SetupAssets()
	{
		bool noAssetList = false;
		// 1. read asset list and check resource valid
		JsonArchive archive(ArchiveMode::ArchiveMode_Read, &mFileSystem);
		if (!archive.OpenJson(COMPILED_PATH_LIST)) {
			noAssetList = true;
		}
		else
		{
			archive.Read("resources", [&](JsonArchive& archive) {
				size_t resCount = archive.GetCurrentValueCount();
				for (int i = 0; i < resCount; i++)
				{
					Path path;
					archive.Read(i, path);
					ResourceType type = ResourceManager::GetResourceType(path.c_str());
					if (type != ResourceType::INVALID_TYPE)
					{
						if (mFileSystem.IsFileExists(path.c_str())) {
							mResources.insert(path.GetHash(), { path, GetResDirHash(path.c_str()), type });
						}
						else
						{
							Path convertedPath = ResourceManager::GetResourceConvertedPath(path);
							if (!convertedPath.IsEmpty()) {
								mFileSystem.DeleteFile(convertedPath.c_str());
							}
						}
					}
				}
			});
		}

		// 2. scan current directory
		U64 lastModTime = noAssetList ? 0 : mFileSystem.GetLastModTime(COMPILED_PATH_LIST);
		ScanDirectory("", lastModTime);
	}

	void AssetCompilerImpl::ScanDirectory(const char* dir, U64 lastModTime)
	{
		auto files = mFileSystem.EnumerateFiles(dir, EnumrateMode_ALL);
		for (const auto path : files)
		{
			// converted_output
			if (path[0] == '.') {
				continue;
			}
			// reserved files
			if (CheckReserved(path)) {
				continue;
			}

			Path fullPath(dir);
			fullPath.AppendPath(path);

			if (mFileSystem.IsDirExists(fullPath.c_str()))
			{
				ScanDirectory(fullPath.c_str(), lastModTime);
			}
			else
			{
				bool needLoad = false;
				U64 fileLastModTime = mFileSystem.GetLastModTime(fullPath.c_str());
				if (fileLastModTime > lastModTime) {
					needLoad = true;
				}
				else if (mResources.find(fullPath.GetHash()) == nullptr) {
					needLoad = true;
				}

				if (needLoad) {
					AddResource(fullPath.c_str());
				}
			}
		}
	}

	bool AssetCompilerImpl::Compile(const ResCompileTask& task)
	{
		if (task.mRes == nullptr) {
			return false;
		}
		
		// do task immediate
		if (!ResourceManager::ConvertResource(task.mRes, task.mRes->GetType(), task.mInPath, true)) {
			return false;
		}

		return !task.mRes->IsFaild();
	}

	ResourceManager::LoadHook::HookResult AssetCompilerImpl::OnBeforeLoad(Resource* res)
	{
		// check res need to convert before load
		Path inPath = res->GetPath();
		Path convertedPath;
		if (!ResourceManager::CheckResourceNeedConvert(inPath, &convertedPath)) {
			return ResourceManager::LoadHook::IMMEDIATE;
		}

		ResCompileTask task;
		task.mRes = res;
		task.mInPath = inPath;
		mToCompileTasks.Enqueue(task);

		Concurrency::AtomicIncrement(&mPendingTasks);
		mCompileSemaphore.Signal(1);

		return ResourceManager::LoadHook::DEFERRED;
	}

	void AssetCompilerImpl::ProcessCompiledTasks()
	{
		while (true)
		{
			ResCompileTask compiledTask;
			if (!mCompiledTasks.Dequeue(compiledTask)) {
				break;
			}

			mLoadHook.ContinueLoad(compiledTask.mRes);
		}
	}

	void AssetCompilerImpl::ProcessChangedFiles()
	{
		bool isFileChanged = false;

		if (isFileChanged) {
			OnListChanged();
		}
	}

	void AssetCompilerImpl::RecoredResources()
	{
		File mOutputFile;
		if (!mFileSystem.OpenFile(TEMP_COMPILED_PATH_LIST, mOutputFile, FileFlags::DEFAULT_WRITE)) {
			Logger::Error("Failed to save asset list file");
		}
		else
		{
			JsonArchive archive(ArchiveMode::ArchiveMode_Write);
			archive.PushMap("resources", [&](JsonArchive& archive) {
				for (const auto kvp : mResources) {
					archive.WriteAndPush(kvp.second.mResPath);
				}
			});

			auto jsonString = archive.DumpJsonString();
			mOutputFile.Write(jsonString.data(), jsonString.size());
			mOutputFile.Close();

			mFileSystem.DeleteFile(COMPILED_PATH_LIST);
			mFileSystem.MoveFile(TEMP_COMPILED_PATH_LIST, COMPILED_PATH_LIST);
		}
	}

	void AssetCompilerImpl::OnFileChanged(const char* path)
	{
		if (StringUtils::StartsWithPrefix(path, COMPILED_PATH_NAME)) {
			return;
		}

		if (CheckReserved(path)) {
			return;
		}

		mChangedFiles.push(Path(path));
	}

	void AssetCompilerImpl::AddResource(const char* fullPath)
	{
		// TODO
		ResourceType type = ResourceManager::GetResourceType(fullPath);
		if (type != ResourceType::INVALID_TYPE) {
			AddResource(type, fullPath);
		}
	}

	void AssetCompilerImpl::AddResource(ResourceType type, const char* path)
	{
		const U32 hash = Path(path).GetHash();
		Concurrency::ScopedReadLock lock(mRWLock);
		auto it = mResources.find(hash);
		if (it != nullptr) {
			it->mResType = type;
		}
		else
		{
			mResources.insert(hash, {Path(path), GetResDirHash(path), type});
		}
	}

	/// //////////////////////////////////////////////////////////////////////////////////
	/// AssetCompiler
	AssetCompiler::AssetCompiler(GameEditor& gameEditor)
	{
		mImpl = CJING_NEW(AssetCompilerImpl)(gameEditor);
	}

	AssetCompiler::~AssetCompiler()
	{
		mImpl->RecoredResources();
		CJING_SAFE_DELETE(mImpl);
	}

	void AssetCompiler::SetupAssets()
	{
		mImpl->SetupAssets();
	}

	void AssetCompiler::Update(F32 deltaTime)
	{
		// process compiled tasks
		mImpl->ProcessCompiledTasks();
		// process changed files
		mImpl->ProcessChangedFiles();
	}

	void AssetCompiler::CompileResources()
	{
		// TODO
		// compile all resources
		for (const auto& kvp : mImpl->mResources)
		{
			Path convertedPath;
			if (!ResourceManager::CheckResourceNeedConvert(kvp.second.mResPath, &convertedPath)) {
				continue;
			}

			ResCompileTask task;
			task.mRes = nullptr;
			task.mInPath = kvp.second.mResPath;
			mImpl->mToCompileTasks.Enqueue(task);

			Concurrency::AtomicIncrement(&mImpl->mPendingTasks);
			mImpl->mCompileSemaphore.Signal(1);
		}
	}

	Signal<void()>& AssetCompiler::GetOnListChanged()
	{
		return mImpl->OnListChanged;
	}

	const HashMap<U32, AssetCompiler::ResourceItem>& AssetCompiler::MapResources()const
	{
		mImpl->mRWLock.BeginWrite();
		return mImpl->mResources;
	}

	void AssetCompiler::UnmapResources()
	{
		mImpl->mRWLock.EndWrite();
	}
}