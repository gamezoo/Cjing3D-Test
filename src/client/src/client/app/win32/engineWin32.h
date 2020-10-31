#pragma once

#ifdef CJING3D_PLATFORM_WIN32

#include "client\app\engine.h"
#include "core\signal\eventQueue.h"

namespace Cjing3D::Win32
{
	class GameWindowWin32;

	class EngineWin32 : public Engine
	{
	public:
		EngineWin32(SharedPtr<GameWindowWin32> gameWindow, PresentConfig& config);
		~EngineWin32();

		void Initialize()override;
		void Uninitialize()override;

		void SetAssetPath(const String& path, const String& name);
		void SetSystemEventQueue(const SharedPtr<EventQueue>& eventQueue);
		void HandleSystemMessage(const Event& systemEvent);
		void DoSystemEvents()override;

	private:
		String mAssetName = "";
		String mAssetPath = "Assets";

		bool mIsLockFrameRate = false;
		U32 mTargetFrameRate = 60;
		I32x2 mScreenSize = I32x2(0, 0);
		SharedPtr<GameWindowWin32> mGameWindowWin32 = nullptr;

		ScopedConnection mSystemConnection;
		SharedPtr<EventQueue> mSystemEventQueue = nullptr;
	};
}

#endif