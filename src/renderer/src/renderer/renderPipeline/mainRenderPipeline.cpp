#include "mainRenderPipeline.h"
#include "renderer\renderer.h"
#include "renderer\renderGraph.h"
#include "gpu\commandList.h"
#include "gpu\gpu.h"

namespace Cjing3D
{

	/// ///////////////////////////////////////////////////////////////////
	// Render passes
	static void AddFullscreenPass(RenderGraph& graph, RenderGraphResource outColor)
	{
		graph.AddCallbackRenderPass("FullScreenPass",
			[&](RenderGraphResBuilder& builder) {

				builder.AddRTV(outColor, RenderGraphFrameAttachment::RenderTarget(
						GPU::BindingFrameAttachment::LOAD_CLEAR
					));
			},
			[&](RenderGraphResources& resources, GPU::CommandList& cmd) {
				auto fbs = resources.GetFrameBindingSet();
				if (fbs == GPU::ResHandle::INVALID_HANDLE) {
					return;
				}

				auto shader = Renderer::GetShader(SHADERTYPE_MAIN);
				if (!shader) {
					return;
				}

				ShaderTechniqueDesc desc = {};
				desc.mPrimitiveTopology = GPU::TRIANGLESTRIP;
				auto tech = shader->CreateTechnique("TECH_OBJECT", desc);
				cmd.BeginFrameBindingSet(fbs);
				{
					auto pipelineState = tech.GetPipelineState();
					cmd.BindPipelineState(pipelineState);
					cmd.Draw(3, 0);
				}
				cmd.EndFrameBindingSet();
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
		AddFullscreenPass(graph, GetResource("rtMain"));
	}
}