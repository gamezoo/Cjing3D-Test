#pragma once

#include "renderPath.h"
#include "renderer\renderGraph.h"
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
		void Stop()override;
		void Update(F32 dt)override;
		void Render()override;
		void Compose(GPU::CommandList& cmd)override;

		virtual void UpdatePipelines() {};
		void AddFinalResource(RenderGraphResource res);
		RenderGraph& GetRenderGraph() { return mMainGraph; }

	protected:
		RenderGraph mMainGraph;
		DynamicArray<RenderGraphResource> mFinalResources;
	};
}