#pragma once

#include "renderer\renderPipeline\renderPipeline.h"

namespace Cjing3D
{
	class ImGuiPipeline : public RenderPipeline
	{
	public:
		ImGuiPipeline();
		virtual ~ImGuiPipeline();

		void Setup(RenderGraph& graph);
	};
}