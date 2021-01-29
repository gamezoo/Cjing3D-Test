#pragma once

#include "renderPath.h"
#include "renderer\renderer.h"
#include "renderer\renderGraph.h"
#include "renderer\texture.h"
#include "core\event\eventSystem.h"
#include "renderer\renderPipeline\mainRenderPipeline.h"

namespace Cjing3D
{
	class RenderGraphPath : public RenderPath
	{
	public:
		RenderGraphPath();
		virtual ~RenderGraphPath();

		void Start()override;
		void Stop()override;
		void Update(F32 dt)override;
		void Render()override;
		void Compose(GPU::CommandList& cmd)override;

		void Clear();
		void ResizeBuffers();

	private:
		RenderGraph mMainGraph;
		MainRenderPipeline mMainPipeline;
		ScopedConnection mResolutionChangedHandle;

		Texture mRTMain;

		Viewport mViewport;
		CullResult mVisibility;
		FrameCB mFrameCB;
	};
}