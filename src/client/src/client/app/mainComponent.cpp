#include "mainComponent.h"
#include "core\helper\timer.h"
#include "core\helper\profiler.h"
#include "resource\resourceManager.h"

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

		Timer::Instance().Start();
		mDeltaTimeAccumulator = 0;

		OnLoad();

		ResourceManager::WaitAll();
		mIsInitialized = true;
	}

	void MainComponent::Uninitialize()
	{
		if (!mIsInitialized == true) {
			return;
		}

		OnUnload();

		Timer::Instance().Stop();
		mIsInitialized = false;
	}

	void MainComponent::SetRenderPath(const SharedPtr<RenderPath>& renderPath)
	{
		mNextRenderPath = renderPath;
	}

	void MainComponent::Tick()
	{
		Profiler::BeginFrame();

		auto engineTime = Timer::Instance().GetTime();
		F32 deltaTime = engineTime.GetDeltaTime();
		 
		const auto& initConfig = mEngine->GetInitConfig();
		U32 targetFrameRate = initConfig.mTargetFrameRate;
		U32 isLockFrameRate = initConfig.mIsLockFrameRate;

		if (mGameWindow->IsWindowActive())
		{
			// fixed update
			const F32 dt = isLockFrameRate ? (1.0f / targetFrameRate) : deltaTime;
			if (!mIsSkipFrame)
			{
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
					FixedUpdate();
					mDeltaTimeAccumulator -= targetRateInv;
				}
			}

			Update(dt);
			Render();
		}
		else
		{
			mDeltaTimeAccumulator = 0;
		}

		Compose();

		Profiler::EndFrame();
	}

	void MainComponent::FixedUpdate()
	{
		PROFILER_CPU_BLOCK("FiexdUpdate");
		mEngine->FixedUpdate();
	}

	void MainComponent::Update(F32 deltaTime)
	{
		PROFILER_CPU_BLOCK("Update");
		mEngine->Update(deltaTime);
	}

	void MainComponent::Render()
	{
	}

	void MainComponent::Compose()
	{
	}
}