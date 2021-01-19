#pragma once

#include "renderPipeline.h"

namespace Cjing3D
{
	class MainRenderPipeline : public RenderPipeline
	{
	public:
		MainRenderPipeline();
		virtual ~MainRenderPipeline();

		void Setup(RenderGraph& graph)override;
	};
}