#include "gameAppWin32.h"
#include "client\app\win32\gameWindowWin32.h"
#include "client\app\win32\engineWin32.h"
#include "client\app\mainComponent.h"
#include "core\container\map.h"
#include "core\concurrency\jobsystem.h"

namespace Cjing3D::Win32
{
	class DebugLoggerSink : public LoggerSink
	{
	public:
		void Log(LogLevel level, const char* msg)override
		{
			if (level == LogLevel::LVL_ERROR) {
				Debug::DebugOuput("Error:");
			}
			Debug::DebugOuput(msg);
			Debug::DebugOuput("\n");
		}
	};
	static StdoutLoggerSink mStdoutLoggerSink;
	static DebugLoggerSink  mDebugLoggerSink;

	class GameAppWinImpl
	{
	public:
		HINSTANCE mHinstance;
		SharedPtr<EventQueue> mEventQueue;
		SharedPtr<GameWindowWin32> mGameWindow;
		SharedPtr<EngineWin32> mEngine;
		SharedPtr<MainComponent> mMainComponent;

	public:
		GameAppWinImpl(HINSTANCE hInstance) :
			mHinstance(hInstance) 
		{
			// init profiler
			Profiler::Initialize();
			Profiler::SetCurrentThreadName("MainThread");

			// init jobsystme
			JobSystem::Initialize(Platform::GetCPUsCount(), JobSystem::MAX_FIBER_COUNT, JobSystem::FIBER_STACK_SIZE);
		}

		~GameAppWinImpl()
		{
			// uninit jobsystem
			JobSystem::Uninitialize();

			// uninit profiler
			Profiler::Uninitilize();
		}

		void Initialize(InitConfig config, GameAppWin32::CreateMainComponentFunc mainCompCreateFunc = nullptr)
		{
			if (config.mTitle == nullptr) {
				config.mTitle = (String("Cjing3D ") + CjingVersion::GetVersionString()).c_str();
			}

			Debug::SetAbortOnDie(true);
#ifdef DEBUG
			Debug::SetDebugConsoleEnable(true);
			Debug::SetDieOnError(true);

			///////////////////////////////////////////////////////////////////////////////
			// show debug console
			if (config.mFlag & PresentFlag_WinApp &&
				Debug::IsDebugConsoleEnable())
			{
				// console for std output..
				if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
					AllocConsole();
				}

				freopen("CONIN$", "r", stdin);
				freopen("CONOUT$", "w", stdout);
				freopen("CONOUT$", "w", stderr);
			}
			Logger::RegisterSink(mStdoutLoggerSink);
			Logger::RegisterSink(mDebugLoggerSink);

			Logger::SetIsDisplayTime(false);
			Logger::Print(CjingVersion::GetHeaderString());
			Logger::SetIsDisplayTime(true);
#endif

			// system event queue
			mEventQueue = CJING_MAKE_SHARED<EventQueue>();
			// create game window
			mGameWindow = CJING_MAKE_SHARED<GameWindowWin32>(mHinstance, config.mTitle, mEventQueue, config);
			// create game engine
			mEngine = CJING_MAKE_SHARED<EngineWin32>(mGameWindow, config);
			mEngine->SetSystemEventQueue(mEventQueue);

			// create main component and preInitialize before engine
			SharedPtr<MainComponent> mainComponent = nullptr;
			if (mainCompCreateFunc != nullptr)
			{
				Debug::CheckAssertion(mainCompCreateFunc != nullptr);
				mainComponent = mainCompCreateFunc(mEngine);
				if (mainComponent == nullptr) {
					Debug::Die("Failed to create main component.");
				}
				mainComponent->PreInitialize();
			}
			mMainComponent = mainComponent;

			// initialize engine
			mEngine->Initialize();

			// initialize main component
			if (mMainComponent != nullptr) {
				mMainComponent->Initialize();
			}
		}

		void Update()
		{
			while (mGameWindow->Tick())
			{
				if (mGameWindow->IsExiting()) {
					break;
				}
				mEngine->DoSystemEvents();

				if (mMainComponent != nullptr) {
					mMainComponent->Tick();
				}
			}
		}
		
		void Uninitialize()
		{
			if (mMainComponent != nullptr)
			{
				mMainComponent->Uninitialize();
				mMainComponent.reset();
			}

			mEngine->Uninitialize();
			mEngine.reset();

			mEventQueue.reset();
			mGameWindow.reset();
		}
	};

	GameAppWin32::GameAppWin32(HINSTANCE hInstance) :
		mHinstance(hInstance)
	{
	}

	void GameAppWin32::Run(InitConfig config, CreateMainComponentFunc mainCompCreateFunc)
	{
		GameAppWinImpl gameApp(mHinstance);
		Concurrency::Semaphore semaphore(0, 1, "MainThreadSem");

		JobSystem::RunJob([config, mainCompCreateFunc, &semaphore](I32 index, void* data) {
			GameAppWinImpl* app = reinterpret_cast<GameAppWinImpl*>(data);
			app->Initialize(config, mainCompCreateFunc);
			app->Update();
			app->Uninitialize();

			semaphore.Signal(1);
		}, this, nullptr, JobSystem::Priority::NORMAL, 0, "MainThreadJob");

		PROFILE_CPU_BLOCK("Sleeping");
		semaphore.Wait();
	}
}