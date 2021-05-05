#pragma once

#include "renderPath.h"
#include "renderer\renderGraph\renderGraph.h"
#include "renderer\texture.h"
#include "core\event\eventSystem.h"

namespace Cjing3D
{
	class RenderGraphPath : public RenderPath
	{
	public:
		RenderGraphPath();
		virtual ~RenderGraphPath();

		void Start()override;
		void Update(F32 dt)override;
		void Stop()override;
		void Render()override;
		void Compose(GPU::ResHandle rtHandle, const GPU::TextureDesc& rtDesc)override;

		virtual void ResizeBuffers(const U32x2& resolution) = 0;
		virtual void RenderPipelines(RenderGraph& renderGraph) = 0;		// called by RenderGraph::Render
		virtual void ComposePipelines(GPU::CommandList& cmd) = 0; 		// called by RenderGraph::Compose

	protected:
		void AddFinalResource(const RenderGraphResource& res);

	private:
		U32x2 mCurrentBufferSize = U32x2(0u, 0u);
		RenderGraph mMainGraph;
		DynamicArray<RenderGraphResource> mFinalResources;
	};
}