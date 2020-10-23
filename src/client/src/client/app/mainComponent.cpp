#include "mainComponent.h"
#include "core\helper\timer.h"

namespace Cjing3D
{
	MainComponent::MainComponent(const SharedPtr<Engine>& engine) :
		mEngine(engine),
		mGameWindow(engine->GetGameWindow())
	{
	}

	MainComponent::~MainComponent()
	{
	}

	void MainComponent::Initialize()
	{
		if (mIsInitialized == true) {
			return;
		}


		mIsInitialized = true;
	}

	void MainComponent::Uninitialize()
	{
		if (!mIsInitialized == true) {
			return;
		}

		mIsInitialized = false;
	}

	void MainComponent::SetRenderPath(const SharedPtr<RenderPath>& renderPath)
	{
		mNextRenderPath = renderPath;
	}

	void MainComponent::Tick()
	{
		auto engineTime = Timer::Instance().GetTime();
		F32 deltaTime = engineTime.GetDeltaTime();
		 
		const auto& presentConfig = mEngine->GetPresentConfig();
		U32 targetFrameRate = presentConfig.mTargetFrameRate;
		U32 isLockFrameRate = presentConfig.mIsLockFrameRate;

		if (mGameWindow->IsWindowActive())
		{
			const F32 dt = isLockFrameRate ? (1.0f / targetFrameRate) : deltaTime;
			if (!mIsSkipFrame)
			{
				DoSystemEvents();
				FixedUpdate();
			}
			else
			{
				mDeltaTimeAccumulator += dt;

				// 某些情况会触发超时操作，此时需要重置计数
				if (mDeltaTimeAccumulator > 8) {
					mDeltaTimeAccumulator = 0;
				}

				const F32 targetRateInv = 1.0f / targetFrameRate;
				while (mDeltaTimeAccumulator >= targetRateInv)
				{
					DoSystemEvents();
					FixedUpdate();
					mDeltaTimeAccumulator -= targetRateInv;
				}
			}
			Update(dt);
			UpdateInput(dt);

			PreRender();
			Render();
			PostRender();
		}
		else
		{
			mDeltaTimeAccumulator = 0;
			UpdateInput(deltaTime);
		}

		Compose();
		EndFrame();
	}

	void MainComponent::FixedUpdate()
	{
	}

	void MainComponent::Update(F32 deltaTime)
	{
	}

	void MainComponent::UpdateInput(F32 deltaTime)
	{
	}

	void MainComponent::PreRender()
	{
	}

	void MainComponent::Render()
	{
	}

	void MainComponent::PostRender()
	{
	}

	void MainComponent::Compose()
	{
	}

	void MainComponent::EndFrame()
	{
	}

	void MainComponent::DoSystemEvents()
	{
		mEngine->DoSystemEvents();
	}
}