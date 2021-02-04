#pragma once

#include "renderPipeline.h"
#include "renderer\cullingSystem.h"

namespace Cjing3D
{
	class MainRenderPipeline : public RenderPipeline
	{
	public:
		MainRenderPipeline();
		virtual ~MainRenderPipeline();

		void Setup(RenderGraph& graph, const Viewport& viewport, FrameCB& frameCB, const CullResult& cullResult);
	};
}