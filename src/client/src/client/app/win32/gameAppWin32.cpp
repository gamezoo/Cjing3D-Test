#include "gameAppWin32.h"
#include "client\app\win32\gameWindowWin32.h"
#include "client\app\win32\engineWin32.h"
#include "client\app\mainComponent.h"

namespace Cjing3D::Win32
{
	GameAppWin32::GameAppWin32()
	{
	}

	void GameAppWin32::SetInstance(HINSTANCE hInstance)
	{
		mHinstance = hInstance;
	}

	void GameAppWin32::SetAssetPath(const String& path, const String& name)
	{
		mAssetPath = path;
		mAssetName = name;
	}

	void GameAppWin32::SetTitleName(const UTF8String& string)
	{
		mTitleName = string;
	}

	void GameAppWin32::SetScreenSize(const I32x2& screenSize)
	{
		mScreenSize = screenSize;
	}

	void GameAppWin32::Run(const CreateGameFunc& createGame)
	{
#ifdef DEBUG
		Debug::SetDebugConsoleEnable(true);
		Debug::SetDieOnError(true);
#endif
		Logger::PrintConsoleHeader();

		PresentConfig config = {};
		config.mScreenSize = mScreenSize;
		config.mIsFullScreen = false;
		config.mIsLockFrameRate = true;
		config.mTargetFrameRate = 60;
		//config.mBackBufferFormat = FORMAT_R8G8B8A8_UNORM;
	
		if (mTitleName == "") {
			mTitleName = String("Cjing3D ") + CjingVersion::GetVersionString();
		}

		// system event queue
		SharedPtr<EventQueue> eventQueue = CJING_MAKE_SHARED<EventQueue>();

		// create game window
		auto gameWindow = CJING_MAKE_SHARED<GameWindowWin32>(mHinstance, mTitleName, eventQueue, config);

		// create game engine
		auto engine = CJING_MAKE_SHARED<EngineWin32>(gameWindow, config);
		engine->SetAssetPath(mAssetPath, mAssetName);
		engine->SetSystemEventQueue(eventQueue);
		engine->Initialize();

		// create game
		Debug::CheckAssertion(createGame != nullptr);
		auto game = createGame(engine);
		if (game == nullptr) {
			Debug::Die("Failed to create game.");
		}
		game->Initialize();

		// run game
		while (gameWindow->Tick()) 
		{
			if (game->GetIsExiting()) {
				break;
			}
			game->Tick();
		} 

		// uninitialize
		game->Uninitialize();
		engine->Uninitialize();
	}

	void GameAppWin32::Run()
	{
		PresentConfig config = {};
		config.mScreenSize = mScreenSize;
		config.mIsFullScreen = false;
		config.mIsLockFrameRate = true;
		config.mTargetFrameRate = 60;
		config.mFlag |= PresentFlag_WinApp;
		//config.mBackBufferFormat = FORMAT_R8G8B8A8_UNORM;

		if (mTitleName == "") {
			mTitleName = String("Cjing3D ") + CjingVersion::GetVersionString();
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
			if (!AttachConsole(ATTACH_PARENT_PROCESS))
			{
				AllocConsole();
			}

			freopen("CONIN$", "r", stdin);
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);
		}
#endif
		// print console header
		Logger::PrintConsoleHeader();

		// system event queue
		SharedPtr<EventQueue> eventQueue = CJING_MAKE_SHARED<EventQueue>();

		// create game window
		auto gameWindow = CJING_MAKE_SHARED<GameWindowWin32>(mHinstance, mTitleName, eventQueue, config);

		// run game
		while (gameWindow->Tick()) {}
	}
}