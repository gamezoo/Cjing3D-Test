#include "mainRenderPipeline.h"
#include "renderer\renderer.h"
#include "renderer\renderGraph.h"
#include "gpu\commandList.h"
#include "gpu\gpu.h"

namespace Cjing3D
{

	/// ///////////////////////////////////////////////////////////////////
	// Render passes
	struct RenderFramesData
	{
		RenderGraphResource mResFrameCB;
		RenderGraphResource mResCameraCB;
	};

	static RenderFramesData AddSetupRenderFramesPass(RenderGraph& graph, FrameCB& frameCB, const Viewport& viewport)
	{
		auto resFrameCB = graph.ImportBuffer("FrameCB", Renderer::GetConstantBuffer(CBTYPE_FRAME));
		auto resCameraCB = graph.ImportBuffer("CameraCB", Renderer::GetConstantBuffer(CBTYPE_CAMERA));

		struct SetupRenderFramesData
		{
			FrameCB mFrameCB;
			CameraCB mCameraCB;

			RenderGraphResource mResFrameCB;
			RenderGraphResource mResCameraCB;
		};
		auto& renderPass = graph.AddDataRenderPass<SetupRenderFramesData>("SetupRenderFramesPass",
			[&](RenderGraphResBuilder& builder, SetupRenderFramesData& data) {
				
				data.mFrameCB = frameCB;
				builder.AddInput(resFrameCB, GPU::BIND_FLAG::BIND_CONSTANT_BUFFER);

				Renderer::UpdateCameraCB(viewport, data.mCameraCB);
				builder.AddInput(resCameraCB, GPU::BIND_FLAG::BIND_CONSTANT_BUFFER);
			},
			[&](RenderGraphResources& resources, GPU::CommandList& cmd, SetupRenderFramesData& data) {
				cmd.UpdateBuffer(resources.GetBuffer(data.mResFrameCB), &data.mFrameCB, 0, sizeof(FrameCB));
				cmd.UpdateBuffer(resources.GetBuffer(data.mResCameraCB), &data.mCameraCB, 0, sizeof(CameraCB));
			}
		);

		RenderFramesData framesData;
		framesData.mResCameraCB = renderPass.GetData().mResCameraCB;
		framesData.mResFrameCB = renderPass.GetData().mResFrameCB;
		return framesData;
	}

	static void AddDepthPrepassPass(RenderGraph& graph)
	{
	}

	static void AddLightCullingPass(RenderGraph& graph)
	{
	}

	static void AddShadowMapsPass(RenderGraph& graph)
	{
	}

	static void AddOpaqueScenePass(RenderGraph& graph)
	{
	}

	static void AddTransparentScenePass(RenderGraph& graph)
	{
	}

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

	void MainRenderPipeline::Setup(RenderGraph& graph, const Viewport& viewport, FrameCB& frameCB, const CullResult& cullResult)
	{
		// setup render frames
		auto renderFramesData = AddSetupRenderFramesPass(graph, frameCB, viewport);

		// depth prepass
		AddDepthPrepassPass(graph);

		// light culling
		AddLightCullingPass(graph);

		// shadow maps
		AddShadowMapsPass(graph);

		// opaque scene
		AddOpaqueScenePass(graph);

		// transparent scene
		AddTransparentScenePass(graph);

		// test
		AddFullscreenPass(graph, GetResource("rtMain"));
	}
}