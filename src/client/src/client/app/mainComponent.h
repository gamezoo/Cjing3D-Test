#pragma once

#include "client\common\common.h"
#include "core\engine.h"
#include "gpu\gpu.h"

namespace Cjing3D
{
	class RenderPath;

	class MainComponent
	{
	public:
		MainComponent(const SharedPtr<Engine>& engine);
		virtual ~MainComponent();

		virtual void PreInitialize() {}
		virtual void Initialize();
		virtual void Tick();
		virtual void Uninitialize();

		void SetRenderPath(RenderPath* renderPath);
		RenderPath* GetRenderPath();
		Universe* GetUniverse();
		Engine* GetEngine();

	protected:
		virtual void OnLoad() {}
		virtual void OnUnload() {}
		virtual void FixedUpdate();
		virtual void Update(F32 deltaTime);
		virtual void Render();
		virtual void Compose();

	protected:
		bool mIsInitialized = false;
		bool mIsSkipFrame = true;
		F32  mDeltaTimeAccumulator = 0;

		SharedPtr<Engine>     mEngine = nullptr;
		SharedPtr<GameWindow> mGameWindow = nullptr;

		Universe* mUniverse = nullptr;
		RenderPath* mRenderPath = nullptr;
	};
}