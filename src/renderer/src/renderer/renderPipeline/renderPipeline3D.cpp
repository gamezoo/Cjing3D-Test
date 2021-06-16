#include "renderPipeline3D.h"
#include "renderer\renderer.h"
#include "renderer\renderGraph\renderGraph.h"
#include "renderer\renderPath\renderGraphPath3D.h"
#include "gpu\commandList.h"
#include "gpu\gpu.h"

namespace Cjing3D
{
	//static void AddLightCullingPass(RenderGraph& graph)
	//{
	//}

	//static void AddShadowMapsPass(RenderGraph& graph)
	//{
	//}

	//static void AddOpaqueScenePass(RenderGraph& graph, const Visibility& cullResult, RenderFramesData& framesData, DepthData& depthData, RenderGraphResource rtMain)
	//{
	//	struct OpaqueSceneData
	//	{
	//		GPU::ViewPort mViewport;
	//		Visibility mCullResult;
	//	};

	//	graph.AddDataRenderPass<OpaqueSceneData>("OpaqueScenePass",
	//		[&](RenderGraphResBuilder& builder, OpaqueSceneData& data) {

	//			auto texture = builder.GetTextureDesc(depthData.mOutDepth);
	//			if (texture == nullptr) {
	//				return;
	//			}
	//			data.mViewport.mWidth = (F32)texture->mWidth;
	//			data.mViewport.mHeight = (F32)texture->mHeight;

	//			data.mCullResult = cullResult;

	//			builder.AddRTV(rtMain, RenderGraphAttachment::RenderTarget());
	//			builder.SetDSV(depthData.mOutDepth, RenderGraphAttachment::DepthStencil(
	//				GPU::BindingFrameAttachment::LOAD_DEFAULT
	//			));
	//		},
	//		[&](RenderGraphResources& resources, GPU::CommandList& cmd, OpaqueSceneData& data) {
	//			
	//			auto fbs = resources.GetFrameBindingSet();
	//			if (fbs == GPU::ResHandle::INVALID_HANDLE) {
	//				return;
	//			}

	//			cmd.BindViewport(data.mViewport);
	//			if (auto binding = cmd.BindScopedFrameBindingSet(fbs)) {
	//				Renderer::DrawScene(RENDERPASS_MAIN, RENDERTYPE_OPAQUE, data.mCullResult, resources, cmd);
	//			}
	//		}
	//	);
	//}

	static void AddTransparentScenePass(RenderGraph& graph)
	{
	}

	/// ///////////////////////////////////////////////////////////////////
	// Main pipeline
	RenderPipeline3D::RenderPipeline3D()
	{
	}

	RenderPipeline3D::~RenderPipeline3D()
	{
	}

	void RenderPipeline3D::Setup(RenderGraphPath3D& renderPath, RenderGraph& renderGraph, const Viewport& viewport, const FrameCB& frameCB, const Visibility& visibility)
	{

		// Opaque pass
		renderGraph.AddCallbackRenderPass("OpaquePass",
			RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
			[&](RenderGraphResBuilder& builder) {

				return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {

				};
			});

		// Transparent pass
		renderGraph.AddCallbackRenderPass("TransparentPass",
			RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
			[&](RenderGraphResBuilder& builder) {

				return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {

				};
			});

		// Post process
		renderGraph.AddCallbackRenderPass("Postprocess",
			RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
			[&](RenderGraphResBuilder& builder) {

				return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {

				};
			});

		// setup render frames
		// RenderFramesData renderFramesData = AddSetupRenderFramesPass(graph, frameCB, viewport);

		// depth prepass
		// DepthData depthData =  AddDepthPrepassPass(graph, cullResult, renderFramesData, GetResource("dbMain"));

		// light culling
		//AddLightCullingPass(graph);

		// shadow maps
		//AddShadowMapsPass(graph);

		// opaque scene
		// AddOpaqueScenePass(graph, cullResult, renderFramesData, depthData, GetResource("rtMain"));

		// transparent scene
		// AddTransparentScenePass(graph);

		// post processes
		// AddPostprocessesPass(graph);
	}
}