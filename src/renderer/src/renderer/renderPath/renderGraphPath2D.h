#pragma once

#include "renderGraphPath.h"
#include "renderer\renderPipeline\renderPipeline2D.h"

namespace Cjing3D
{
	class RenderGraphPath2D : public RenderGraphPath
	{
	public:
		RenderGraphPath2D();
		virtual ~RenderGraphPath2D();

		void Start()override;
		void Stop()override;
		void Update(F32 dt)override;
		void ResizeBuffers(const U32x2& resolution)override;
		void RenderPipelines(RenderGraph& renderGraph)override;		// called by RenderGraph::Render
		void ComposePipelines(GPU::CommandList& cmd)override;		// called by RenderGraph::Compose

	protected:
		RenderPipeline2D mRenderPipeline2D;

		Texture mRtMain;
	};
}