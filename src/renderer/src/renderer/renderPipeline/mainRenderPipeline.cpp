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
		struct FullScreenData
		{
			ShaderTechniqueDesc mDesc;
		};

		graph.AddDataRenderPass<FullScreenData>("FullScreenPass",
			[&](RenderGraphResBuilder& builder, FullScreenData& data) {

				ShaderTechniqueDesc desc = {};
				data.mDesc = desc;

				builder.AddRTV(outColor, RenderGraphFrameAttachment::RenderTarget(
						GPU::BindingFrameAttachment::LOAD_CLEAR
					));
			},
			[&](RenderGraphResources& resources, GPU::CommandList& cmd, FullScreenData& data) {
				auto fbs = resources.GetFrameBindingSet();
				if (fbs == GPU::ResHandle::INVALID_HANDLE) {
					return;
				}

				auto shader = Renderer::GetShader(SHADERTYPE_IMAGE);
				if (!shader) {
					return;
				}

				//auto tech = shader->CreateTechnique("TEST_PIPELINE", data.mDesc);
				//cmd.BeginFrameBindingSet(fbs);
				//{
				//	auto pipelineState = tech.GetPipelineState();
				//	cmd.BindPipelineState(pipelineState);
				//	cmd.Draw(3, 0);
				//}
				//cmd.EndFrameBindingSet();
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