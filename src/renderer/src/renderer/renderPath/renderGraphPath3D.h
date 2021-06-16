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
		void ResizeBuffers(const U32x2& resolution)override;
		void RenderPipelines(RenderGraph& renderGraph)override;		// called by RenderGraph::Render
		void ComposePipelines(GPU::CommandList& cmd)override;		// called by RenderGraph::Compose

		constexpr bool IsShadowEnable()const { return mIsShadowEnable; }

	protected:
		virtual void AddFinalResources(RenderGraph& renderGraph);

	protected:
		bool mIsShadowEnable = true;
		
		RenderPipeline3D mRenderPipeline3D;

		Texture mRtMain;
		Texture mDepthMain;

		Viewport mViewport;
		Visibility mVisibility;
		FrameCB mFrameCB;
	};
}