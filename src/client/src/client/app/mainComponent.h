#pragma once

#include "client\common\common.h"
#include "core\engine.h"

namespace Cjing3D
{
	class RenderPath;

	class MainComponent
	{
	public:
		MainComponent(const SharedPtr<Engine>& engine);
		virtual ~MainComponent();

		virtual void Initialize();
		virtual void Tick();
		virtual void Uninitialize();

		void SetRenderPath(const SharedPtr<RenderPath>& renderPath);

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

		SharedPtr<RenderPath> mCurrentRenderPath = nullptr;
		SharedPtr<RenderPath> mNextRenderPath = nullptr;
		SharedPtr<Engine>     mEngine = nullptr;
		SharedPtr<GameWindow> mGameWindow = nullptr;
	};
}