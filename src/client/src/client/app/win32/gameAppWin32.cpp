#include "gameAppWin32.h"
#include "client\app\win32\gameWindowWin32.h"
#include "client\app\win32\engineWin32.h"
#include "client\app\mainComponent.h"
#include "core\container\map.h"

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

	GameAppWin32::GameAppWin32(HINSTANCE hInstance) :
		mHinstance(hInstance)
	{
	}

	void GameAppWin32::Run(InitConfig config, CreateMainComponentFunc mainCompCreateFunc)
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
		SharedPtr<EventQueue> eventQueue = CJING_MAKE_SHARED<EventQueue>();
		
		// create game window
		auto gameWindow = CJING_MAKE_SHARED<GameWindowWin32>(mHinstance, config.mTitle, eventQueue, config);
		
		// create game engine
		auto engine = CJING_MAKE_SHARED<EngineWin32>(gameWindow, config);
		engine->SetSystemEventQueue(eventQueue);
		
		// create main component and preInitialize before engine
		SharedPtr<MainComponent> mainComponent = nullptr;
		if (mainCompCreateFunc != nullptr)
		{
			Debug::CheckAssertion(mainCompCreateFunc != nullptr);
			mainComponent = mainCompCreateFunc(engine);
			if (mainComponent == nullptr) {
				Debug::Die("Failed to create main component.");
			}
			mainComponent->PreInitialize();
		}
			
		// initialize engine
		engine->Initialize();

		// initialize main component
		if (mainComponent != nullptr) {
			mainComponent->Initialize();
		}

		// run game
		while (gameWindow->Tick()) 
		{
			if (gameWindow->IsExiting()) {
				break;
			}
			engine->DoSystemEvents();

			if (mainComponent != nullptr) {
				mainComponent->Tick();
			}
		}

		// uninitialize
		if (mainComponent != nullptr) {
			mainComponent->Uninitialize();
		}

		engine->Uninitialize();
	}
}