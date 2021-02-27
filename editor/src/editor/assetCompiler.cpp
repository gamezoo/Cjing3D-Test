#include "assetCompiler.h"
#include "editor.h"
#include "filesWatcher.h"
#include "resource\resourceManager.h"
#include "core\concurrency\concurrency.h"
#include "core\container\mpmc_bounded_queue.h"
#include "core\serialization\jsonArchive.h"
#include "core\signal\connection.h"

namespace Cjing3D
{
	/// //////////////////////////////////////////////////////////////////////////////////
	/// AssertCompilerImpl
	
	struct ResCompileTask
	{
		Resource* mRes = nullptr;
		Path mInPath;
	};

	class AssertCompilerImpl;
	struct AssertCompilerHook : public ResourceManager::LoadHook
	{
		AssertCompilerHook(AssertCompilerImpl& impl) : mImpl(impl) {}

		virtual HookResult OoBeforeLoad(Resource* res);
		virtual void OnWait();
		AssertCompilerImpl& mImpl;
	};

	class AssertCompilerImpl
	{
	public:
		GameEditor& mGameEditor;
		BaseFileSystem& mFileSystem;
		AssertCompilerHook mLoadHook;
		FilesWatcher* mFilesWatcher = nullptr;
		ScopedConnection mFileChangedConn;

		Concurrency::Thread mCompileThread;
		Concurrency::Semaphore mCompileSemaphore;

		Concurrency::RWLock mRWLock;
		MPMCBoundedQueue<ResCompileTask> mToCompileTasks;
		MPMCBoundedQueue<ResCompileTask> mCompiledTasks;
		volatile I32 mPendingTasks = 0;

		HashMap<U32, AssertCompiler::ResourceItem> mResources;
		Signal<void()> OnListChanged;

		bool mIsExit = false;

	public:
		AssertCompilerImpl(GameEditor& gameEditor);
		~AssertCompilerImpl();

		bool Compile(const ResCompileTask& task);
		ResourceManager::LoadHook::HookResult OnBeforeLoad(Resource* res);
		void ProcessCompiledTasks();
		void RecoredResources();
		void OnFileChanged(const char* path);
		void AddResource(ResourceType type, const char* path);
	};

	ResourceManager::LoadHook::HookResult AssertCompilerHook::OoBeforeLoad(Resource* res)
	{
		return mImpl.OnBeforeLoad(res);
	}

	void AssertCompilerHook::OnWait()
	{
		mImpl.ProcessCompiledTasks();
	}

	static int CompileTaskFunc(void* data)
	{
		auto* impl = reinterpret_cast<AssertCompilerImpl*>(data);
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

	AssertCompilerImpl::AssertCompilerImpl(GameEditor& gameEditor) :
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

		Logger::Info("Assert compiler initialzied");
	}

	AssertCompilerImpl::~AssertCompilerImpl()
	{
		mIsExit = true;

		mFileChangedConn.Disconnect();
		CJING_SAFE_DELETE(mFilesWatcher);

		ResourceManager::SetCurrentLoadHook(nullptr);
		mCompileSemaphore.Signal(1);
		mCompileThread.Join();

		Logger::Info("Assert compiler uninitialized");
	}

	bool AssertCompilerImpl::Compile(const ResCompileTask& task)
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

	ResourceManager::LoadHook::HookResult AssertCompilerImpl::OnBeforeLoad(Resource* res)
	{
		Path inPath = res->GetPath();
		Path convertedPath;
		if (!ResourceManager::CheckResourceNeedConverter(inPath, &convertedPath)) {
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

	void AssertCompilerImpl::ProcessCompiledTasks()
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

	void AssertCompilerImpl::RecoredResources()
	{
		File mOutputFile;
		if (!mFileSystem.OpenFile("converter_output/tmp_list.txt", mOutputFile, FileFlags::DEFAULT_WRITE)) {
			Logger::Error("Failed to save converter_output/_list.txt");
		}
		else
		{
			JsonArchive archive(ArchiveMode::ArchiveMode_Write);
			archive.PushMap("resources", [&](JsonArchive& archive) {
				for (const auto kvp : mResources) {
					archive.WriteAndPush(String(kvp.second.mResPath.c_str()));
				}
			});

			auto jsonString = archive.DumpJsonString();
			mOutputFile.Write(jsonString.data(), jsonString.size());
			mOutputFile.Close();

			mFileSystem.DeleteFile("converter_output/_list.txt");
			mFileSystem.MoveFile("converter_output/tmp_list.txt", "converter_output/_list.txt");
		}
	}

	void AssertCompilerImpl::OnFileChanged(const char* path)
	{
	
	}

	void AssertCompilerImpl::AddResource(ResourceType type, const char* path)
	{
	}

	/// //////////////////////////////////////////////////////////////////////////////////
	/// AssertCompiler
	AssertCompiler::AssertCompiler(GameEditor& gameEditor)
	{
		mImpl = CJING_NEW(AssertCompilerImpl)(gameEditor);
	}

	AssertCompiler::~AssertCompiler()
	{
		mImpl->RecoredResources();
		CJING_SAFE_DELETE(mImpl);
	}

	void AssertCompiler::SetupAsserts()
	{
	}

	void AssertCompiler::Update(F32 deltaTime)
	{
		// process compiled tasks
		mImpl->ProcessCompiledTasks();
	}

	Signal<void()>& AssertCompiler::GetOnListChanged()
	{
		return mImpl->OnListChanged;
	}

	const HashMap<U32, AssertCompiler::ResourceItem>& AssertCompiler::MapResources()const
	{
		mImpl->mRWLock.BeginWrite();
		return mImpl->mResources;
	}

	void AssertCompiler::UnmapResources()
	{
		mImpl->mRWLock.EndWrite();
	}
}