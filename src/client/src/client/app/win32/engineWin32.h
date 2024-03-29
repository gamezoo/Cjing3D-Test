#pragma once

#ifdef CJING3D_PLATFORM_WIN32

#include "core\engine.h"
#include "core\signal\eventQueue.h"

namespace Cjing3D::Win32
{
	class GameWindowWin32;

	class EngineWin32 : public Engine
	{
	public:
		EngineWin32(SharedPtr<GameWindowWin32> gameWindow, InitConfig& config);
		~EngineWin32();

		void Initialize()override;
		void Uninitialize()override;
		void Update(Universe& universe, F32 dt)override;
		void FixedUpdate()override;
		void RequestExit()override;

		void SetSystemEventQueue(const SharedPtr<EventQueue>& eventQueue);
		void HandleSystemMessage(const Event& systemEvent);
		void DoSystemEvents()override;

		BaseFileSystem* GetFileSystem()override;
		InputManager* GetInputManager()override;
		EventQueue* GetEventQueue()override;

	private:
		struct EngineWin32Impl* mImpl = nullptr;
	};
}

#endif