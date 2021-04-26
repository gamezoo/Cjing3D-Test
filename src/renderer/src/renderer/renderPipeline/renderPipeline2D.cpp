#include "renderPipeline2D.h"

namespace Cjing3D
{
	RenderPipeline2D::RenderPipeline2D()
	{
	}

	RenderPipeline2D::~RenderPipeline2D()
	{
	}

	void RenderPipeline2D::Setup(RenderGraph& graph)
	{
		//auto rtMain = GetResource("rtMain");

		//graph.AddCallbackRenderPass("Render2D",
		//	[&](RenderGraphResBuilder& builder) {

		//		builder.AddRTV(rtMain, RenderGraphAttachment::RenderTarget(GPU::BindingFrameAttachment::LOAD_CLEAR));
		//	},
		//	[&](RenderGraphResources& resources, GPU::CommandList& cmd) {
		//		auto fbs = resources.GetFrameBindingSet();
		//		if (fbs == GPU::ResHandle::INVALID_HANDLE) {
		//			return;
		//		}

		//		if (auto binding = cmd.BindScopedFrameBindingSet(fbs)) {
		//			
		//		}
		//	}
		//);
	}
}