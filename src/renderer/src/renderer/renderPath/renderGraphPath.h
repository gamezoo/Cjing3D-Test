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
		void Stop()override;
		void Render()override;

		virtual void UpdatePipelines(RenderGraph& renderGraph) = 0;

	private:
		RenderGraph mMainGraph;
	};
}