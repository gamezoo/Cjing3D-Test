#include "pipeline.h"
#include "manager.h"
#include "renderer\renderPath\renderGraphPath2D.h"

namespace Cjing3D
{
	ImGuiPipeline::ImGuiPipeline()
	{
	}

	ImGuiPipeline::~ImGuiPipeline()
	{
	}

	void ImGuiPipeline::Setup(RenderGraph& graph)
	{
		auto rtRes = graph.GetBloackBoard().Get("rtMain2D");
		if (!rtRes) {
			return;
		}

		graph.AddCallbackRenderPass("ImGuiPass",
			RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
			[&](RenderGraphResBuilder& builder) {

				auto res = builder.AddRTV(rtRes, RenderGraphAttachment::RenderTarget());
				graph.GetBloackBoard().Put("rtMain2D", res);

				return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {						
					ImGuiRHI::Manager::Render(cmd);
				};
			});
	}
}