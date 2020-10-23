#pragma once

#include "client\common\common.h"
#include "client\app\presentConfig.h"
#include "client\platform\gameWindow.h"

namespace Cjing3D
{
	class Engine : public ENABLE_SHARED_FROM_THIS<Engine>
	{
	public:
		Engine(SharedPtr<GameWindow> gameWindow, PresentConfig& presentConfig) : mGameWindow(gameWindow), mPresentConfig(presentConfig){}
		Engine(const Engine& rhs) = delete;
		Engine& operator=(const Engine& rhs) = delete;
		virtual ~Engine() = default;
		
		virtual void Initialize() = 0;
		virtual void Uninitialize() = 0;
		virtual void DoSystemEvents() = 0;

		SharedPtr<GameWindow> GetGameWindow()const {
			return mGameWindow;
		}
		PresentConfig& GetPresentConfig() {
			return mPresentConfig;
		}
		const PresentConfig& GetPresentConfig()const {
			return mPresentConfig;
		}

		void SetIsExiting(bool isExiting) { mIsExiting = isExiting; }
		bool GetIsExiting()const { return mIsExiting; }

	protected:
		SharedPtr<GameWindow> mGameWindow = nullptr;
		PresentConfig& mPresentConfig;
		bool mIsExiting = false;
	};
}
