#pragma once

#include "client\common\common.h"
#include "client\app\initConfig.h"
#include "client\app\gameWindow.h"

namespace Cjing3D
{
	class ModulerPlugin;
	class BaseFileSystem;

	class Engine : public ENABLE_SHARED_FROM_THIS<Engine>
	{
	public:
		Engine(SharedPtr<GameWindow> gameWindow, InitConfig& initConfig) : mGameWindow(gameWindow), mInitConfig(initConfig){}
		Engine(const Engine& rhs) = delete;
		Engine& operator=(const Engine& rhs) = delete;
		virtual ~Engine() = default;
		
		virtual void Initialize() = 0;
		virtual void Uninitialize() = 0;
		virtual void DoSystemEvents() = 0;
		virtual void Update() = 0;

		virtual BaseFileSystem* GetFileSystem() = 0;

		SharedPtr<GameWindow> GetGameWindow()const {
			return mGameWindow;
		}
		InitConfig& GetInitConfig() {
			return mInitConfig;
		}
		const InitConfig& GetInitConfig()const {
			return mInitConfig;
		}
		DynamicArray<ModulerPlugin*>& GetModulerPlugins() { 
			return mModulerPlugins; 
		}

	protected:
		SharedPtr<GameWindow> mGameWindow = nullptr;
		InitConfig& mInitConfig;
		DynamicArray<ModulerPlugin*> mModulerPlugins;
	};
}
