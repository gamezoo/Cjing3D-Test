#include "gameAppWin32.h"
#include "client\app\win32\gameWindowWin32.h"
#include "client\app\win32\engineWin32.h"
#include "client\app\mainComponent.h"

namespace Cjing3D::Win32
{
	GameAppWin32::GameAppWin32(HINSTANCE hInstance) :
		mHinstance(hInstance)
	{
	}

	void GameAppWin32::Run(InitConfig config, CreateGameFunc createGame)
	{
		if (config.mTitle == nullptr) {
			config.mTitle = (String("Cjing3D ") + CjingVersion::GetVersionString()).c_str();
		}

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
		// print console header
		Logger::PrintConsoleHeader();
#endif

		// system event queue
		SharedPtr<EventQueue> eventQueue = CJING_MAKE_SHARED<EventQueue>();
		
		// create game window
		auto gameWindow = CJING_MAKE_SHARED<GameWindowWin32>(mHinstance, config.mTitle, eventQueue, config);
		
		// create game engine
		auto engine = CJING_MAKE_SHARED<EngineWin32>(gameWindow, config);
		engine->SetSystemEventQueue(eventQueue);
		engine->Initialize();

		// create main component
		SharedPtr<MainComponent> mainGame = nullptr;
		if (createGame != nullptr)
		{
			Debug::CheckAssertion(createGame != nullptr);
			mainGame = createGame(engine);
			if (mainGame == nullptr) {
				Debug::Die("Failed to create game.");
			}
			mainGame->Initialize();
		}

		// run game
		while (gameWindow->Tick()) 
		{
			if (gameWindow->IsExiting()) {
				break;
			}
			engine->DoSystemEvents();

			if (mainGame != nullptr) {
				mainGame->Tick();
			}
		}

		// uninitialize
		if (mainGame != nullptr) {
			mainGame->Uninitialize();
		}

		engine->Uninitialize();
	}
}