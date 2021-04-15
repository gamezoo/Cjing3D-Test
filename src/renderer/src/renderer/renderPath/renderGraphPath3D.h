#pragma once

#include "renderGraphPath2D.h"
#include "renderer\renderer.h"
#include "renderer\renderPipeline\renderPipeline3D.h"

namespace Cjing3D
{
	class RenderGraphPath3D : public RenderGraphPath2D
	{
	public:
		RenderGraphPath3D();
		virtual ~RenderGraphPath3D();

		void Start()override;
		void Stop()override;
		void Update(F32 dt)override;
		void UpdatePipelines(RenderGraph& renderGraph)override;
		void Compose(GPU::CommandList& cmd)override;

		void ResizeBuffers();

	protected:
		RenderPipeline3D mRenderPipeline3D;
		ScopedConnection mResolutionChangedHandle;

		Texture mRtMain;
		Texture mDpbMain;

		Viewport mViewport;
		CullingResult mVisibility;
		FrameCB mFrameCB;
	};
}