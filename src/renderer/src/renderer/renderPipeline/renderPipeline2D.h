#pragma once

#include "renderPipeline.h"
#include "renderer\shaderInterop.h"

namespace Cjing3D
{
	class RenderPipeline2D : public RenderPipeline
	{
	public:
		RenderPipeline2D();
		virtual ~RenderPipeline2D();

		void Setup(RenderGraph& graph);
	};
}