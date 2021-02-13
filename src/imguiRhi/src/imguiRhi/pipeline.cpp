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
		RenderGraphResource rtRes = graph.GetResource(RenderGraphPath2D::RT_MAIN_NAME);

		graph.AddCallbackRenderPass("ImGuiPass",
			[&](RenderGraphResBuilder& builder) {

				builder.AddRTV(rtRes, RenderGraphFrameAttachment::RenderTarget());
			},
			[&](RenderGraphResources& resources, GPU::CommandList& cmd) {
				auto fbs = resources.GetFrameBindingSet();
				if (fbs == GPU::ResHandle::INVALID_HANDLE) {
					return;
				}

				if (auto binding = cmd.BindScopedFrameBindingSet(fbs)) {
					ImGuiRHI::Manager::Render(cmd);
				}
			}
		);
	}
}