#include "assetCompiler.h"
#include "editor.h"
#include "resource\resourceManager.h"
#include "core\jobsystem\concurrency.h"
#include "core\container\mpmc_bounded_queue.h"

namespace Cjing3D
{
	/// //////////////////////////////////////////////////////////////////////////////////
	/// AssertCompilerImpl
	class AssertCompilerImpl;
	struct AssertCompilerHook : public ResourceManager::LoadHook
	{
		AssertCompilerHook(AssertCompilerImpl& impl) : mImpl(impl) {}

		virtual HookResult OoBeforeLoad(Resource* res) {
			return HookResult::IMMEDIATE;
		}

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
		MPMCBoundedQueue<Path> mToCompileTasks;
		MPMCBoundedQueue<Path> mCompiledTasks;

		bool mIsExit = false;

	public:
		AssertCompilerImpl(GameEditor& gameEditor);
		~AssertCompilerImpl();

		bool Compile(const Path& path);
	};

	static int CompileTaskFunc(void* data)
	{
		auto* impl = reinterpret_cast<AssertCompilerImpl*>(data);
		while (!impl->mIsExit)
		{
			impl->mCompileSemaphore.Wait();

			Path path;
			if (!impl->mToCompileTasks.Dequeue(path)){
				continue;
			}

			if (!impl->Compile(path)) {
				Debug::Error("Failed to compile resource:%s", path.c_str());
			}

			impl->mCompiledTasks.Enqueue(path);
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

	bool AssertCompilerImpl::Compile(const Path& path)
	{
		return false;
	}

	/// //////////////////////////////////////////////////////////////////////////////////
	/// AssertCompiler
	AssertCompiler::AssertCompiler(GameEditor& gameEditor)
	{
		mImpl = CJING_NEW(AssertCompilerImpl)(gameEditor);
	}

	AssertCompiler::~AssertCompiler()
	{
		CJING_SAFE_DELETE(mImpl);
	}

	void AssertCompiler::SetupAsserts()
	{
	}

	void AssertCompiler::Update(F32 deltaTime)
	{
	}

}