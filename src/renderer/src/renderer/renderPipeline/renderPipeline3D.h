#pragma once

#include "renderPipeline.h"
#include "renderer\shaderInterop.h"

namespace Cjing3D
{
	class RenderPipeline3D : public RenderPipeline
	{
	public:
		RenderPipeline3D();
		virtual ~RenderPipeline3D();

		void Setup(RenderGraph& graph, const Viewport& viewport, FrameCB& frameCB, const CullingResult& cullResult);
	};
}