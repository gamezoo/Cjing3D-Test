#include "mainComponent.h"
#include "core\helper\timer.h"
#include "core\helper\profiler.h"
#include "resource\resourceManager.h"
#include "renderer\renderPath\renderPath.h"
#include "renderer\renderer.h"

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

		// init universe
		mUniverse = mEngine->CreateUniverse();
		Renderer::InitRenderScene(*mEngine, *mUniverse);

		// onLoad
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

		if (mUniverse != nullptr) 
		{
			mEngine->DestroyUniverse(mUniverse);
			mUniverse = nullptr;
		}

		if (mRenderPath != nullptr) 
		{
			mRenderPath->Stop();
			mRenderPath = nullptr;
		}

		Timer::Instance().Stop();
		mIsInitialized = false;
	}

	void MainComponent::SetRenderPath(RenderPath* renderPath)
	{
		if (mRenderPath != nullptr) {
			mRenderPath->Stop();
		}

		mRenderPath = renderPath;

		if (renderPath != nullptr) {
			renderPath->Start();
		}
	}

	RenderPath* MainComponent::GetRenderPath()
	{
		return mRenderPath;
	}

	Universe* MainComponent::GetUniverse()
	{
		return mUniverse;
	}

	void MainComponent::Tick()
	{
		Profiler::BeginFrame();

		auto engineTime = Timer::Instance().GetTime();
		F32 deltaTime = engineTime.GetDeltaTime();
		 
		const auto& initConfig = mEngine->GetInitConfig();
		U32 targetFrameRate = initConfig.mTargetFrameRate;
		U32 isLockFrameRate = initConfig.mIsLockFrameRate;

		bool isActive = mGameWindow->IsWindowActive();
#if DEBUG
		isActive = true;
#endif
		if (!isActive) {
			mDeltaTimeAccumulator = 0;
		}
		else {
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

		Compose();

		Renderer::EndFrame();
		Profiler::EndFrame();
	}

	void MainComponent::FixedUpdate()
	{
		PROFILER_CPU_BLOCK("FiexdUpdate");
		mEngine->FixedUpdate();

		if (mRenderPath != nullptr) {
			mRenderPath->FixedUpdate();
		}
	}

	void MainComponent::Update(F32 deltaTime)
	{
		PROFILER_CPU_BLOCK("Update");
		mEngine->Update(*mUniverse, deltaTime);

		if (mRenderPath != nullptr) {
			mRenderPath->Update(deltaTime);
		}
	}

	void MainComponent::Render()
	{
		if (mRenderPath != nullptr) {
			mRenderPath->Render();
		}
	}

	void MainComponent::Compose()
	{
		GPU::CommandList* cmd = GPU::CreateCommandlist();
		Renderer::PresentBegin(*cmd);
		{
			if (mRenderPath != nullptr) {
				mRenderPath->Compose(*cmd);
			}
		}
		Renderer::PresentEnd();
	}

}