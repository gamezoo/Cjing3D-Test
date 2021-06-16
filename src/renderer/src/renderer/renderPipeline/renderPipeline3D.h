#pragma once

#include "renderPipeline.h"
#include "renderer\shaderInterop.h"

namespace Cjing3D
{
	class RenderGraphPath3D;

	class RenderPipeline3D : public RenderPipeline
	{
	public:
		RenderPipeline3D();
		virtual ~RenderPipeline3D();

		void Setup(RenderGraphPath3D& renderPath, RenderGraph& renderGraph, const Viewport& viewport, const FrameCB& frameCB, const Visibility& cullResult);
	};
}