#include "assetCompiler.h"
#include "editor.h"
#include "resource\resourceManager.h"
#include "core\concurrency\concurrency.h"
#include "core\container\mpmc_bounded_queue.h"

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

		Concurrency::Thread mCompileThread;
		Concurrency::Semaphore mCompileSemaphore;

		Concurrency::RWLock mRWLock;
		MPMCBoundedQueue<ResCompileTask> mToCompileTasks;
		MPMCBoundedQueue<ResCompileTask> mCompiledTasks;
		volatile I32 mPendingTasks = 0;

		bool mIsExit = false;

	public:
		AssertCompilerImpl(GameEditor& gameEditor);
		~AssertCompilerImpl();

		bool Compile(const ResCompileTask& task);
		ResourceManager::LoadHook::HookResult OnBeforeLoad(Resource* res);
		void ProcessCompiledTasks();
		void RecoredResources();
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
		Logger::Info("Assert compiler initialzied");
	}

	AssertCompilerImpl::~AssertCompilerImpl()
	{
		mIsExit = true;
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
}