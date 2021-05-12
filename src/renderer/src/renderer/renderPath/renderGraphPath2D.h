#pragma once

#include "renderGraphPath.h"

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
		virtual void AddFinalResources(RenderGraph& renderGraph);

	protected:
		Texture mRtMain;
	};
}