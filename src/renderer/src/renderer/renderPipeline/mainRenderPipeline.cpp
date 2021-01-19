#include "mainRenderPipeline.h"
#include "renderer\renderGraph.h"
#include "gpu\commandList.h"
#include "gpu\gpu.h"

namespace Cjing3D
{
	/// ///////////////////////////////////////////////////////////////////
	// Render passes
	static void AddFullscreenPass(RenderGraph& graph, MainRenderPipeline& pipeline)
	{
		auto rtMain = pipeline.GetResource("rtMain");
		if (!rtMain) {
			return;
		}

		graph.AddCallbackRenderPass("FullScreenPass",
			[&](RenderGraphResBuilder& builder) {

				builder.AddRTV(rtMain, RenderGraphFrameAttachment::RenderTarget(
						GPU::BindingFrameAttachment::LOAD_CLEAR
					));
			},
			[&](RenderGraphResources& resources, GPU::CommandList& cmd) {
				
			}
		);
	}

	/// ///////////////////////////////////////////////////////////////////
	// Main pipeline
	MainRenderPipeline::MainRenderPipeline()
	{
	}

	MainRenderPipeline::~MainRenderPipeline()
	{
	}

	void MainRenderPipeline::Setup(RenderGraph& graph)
	{
		AddFullscreenPass(graph, *this);
	}
}