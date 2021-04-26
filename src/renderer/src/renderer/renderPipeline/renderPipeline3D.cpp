#include "renderPipeline3D.h"
#include "renderer\renderer.h"
#include "renderer\renderGraph\renderGraph.h"
#include "gpu\commandList.h"
#include "gpu\gpu.h"

namespace Cjing3D
{

	///// ///////////////////////////////////////////////////////////////////
	//// Render passes
	//struct RenderFramesData
	//{
	//	RenderGraphResource mResFrameCB;
	//	RenderGraphResource mResCameraCB;
	//};

	//struct DepthData
	//{
	//	RenderGraphResource mOutDepth;
	//};

	//static RenderFramesData AddSetupRenderFramesPass(RenderGraph& graph, FrameCB& frameCB, const Viewport& viewport)
	//{
	//	auto resFrameCB = graph.ImportBuffer("FrameCB", Renderer::GetConstantBuffer(CBTYPE_FRAME));
	//	auto resCameraCB = graph.ImportBuffer("CameraCB", Renderer::GetConstantBuffer(CBTYPE_CAMERA));

	//	struct SetupRenderFramesData
	//	{
	//		FrameCB mFrameCB;
	//		CameraCB mCameraCB;

	//		RenderGraphResource mResFrameCB;
	//		RenderGraphResource mResCameraCB;
	//	};
	//	auto& renderPass = graph.AddDataRenderPass<SetupRenderFramesData>("SetupRenderFramesPass",
	//		[&](RenderGraphResBuilder& builder, SetupRenderFramesData& data) {
	//			
	//			data.mFrameCB = frameCB;
	//			builder.AddInput(resFrameCB, GPU::BIND_FLAG::BIND_CONSTANT_BUFFER);

	//			Renderer::UpdateCameraCB(viewport, data.mCameraCB);
	//			builder.AddInput(resCameraCB, GPU::BIND_FLAG::BIND_CONSTANT_BUFFER);
	//		},
	//		[&](RenderGraphResources& resources, GPU::CommandList& cmd, SetupRenderFramesData& data) {
	//			cmd.UpdateBuffer(resources.GetBuffer(data.mResFrameCB), &data.mFrameCB, 0, sizeof(FrameCB));
	//			cmd.UpdateBuffer(resources.GetBuffer(data.mResCameraCB), &data.mCameraCB, 0, sizeof(CameraCB));
	//		}
	//	);

	//	RenderFramesData framesData;
	//	framesData.mResCameraCB = renderPass.GetData().mResCameraCB;
	//	framesData.mResFrameCB = renderPass.GetData().mResFrameCB;
	//	return framesData;
	//}

	//static DepthData AddDepthPrepassPass(RenderGraph& graph, const CullingResult& cullResult, RenderFramesData& framesData, RenderGraphResource depthBufferMain)
	//{
	//	struct DepthPrepassData
	//	{
	//		GPU::ViewPort mViewport;
	//		RenderGraphResource mOutDepth;
	//	};

	//	auto& depthPass = graph.AddDataRenderPass<DepthPrepassData>("DepthPrepass",
	//		[&](RenderGraphResBuilder& builder, DepthPrepassData& data) {

	//			auto texture = builder.GetTextureDesc(depthBufferMain);
	//			if (texture == nullptr) {
	//				return;
	//			}

	//			data.mViewport.mWidth = (F32)texture->mWidth;
	//			data.mViewport.mHeight = (F32)texture->mHeight;
	//			
	//			data.mOutDepth = builder.SetDSV(depthBufferMain, RenderGraphAttachment::DepthStencil(
	//				GPU::BindingFrameAttachment::LOAD_CLEAR
	//			));
	//		},
	//		[&](RenderGraphResources& resources, GPU::CommandList& cmd, DepthPrepassData& data) 
	//		{	
	//			auto fbs = resources.GetFrameBindingSet();
	//			if (fbs == GPU::ResHandle::INVALID_HANDLE) {
	//				return;
	//			}
	//			
	//			cmd.BindViewport(data.mViewport);
	//			if (auto binding = cmd.BindScopedFrameBindingSet(fbs)) {
	//				Renderer::DrawScene(RENDERPASS_PREPASS, RENDERTYPE_OPAQUE, cullResult, resources, cmd);
	//			}
	//		}
	//	);

	//	DepthData depthData;
	//	depthData.mOutDepth = depthPass.GetData().mOutDepth;

	//	return depthData;
	//}

	//static void AddLightCullingPass(RenderGraph& graph)
	//{
	//}

	//static void AddShadowMapsPass(RenderGraph& graph)
	//{
	//}

	//static void AddOpaqueScenePass(RenderGraph& graph, const CullingResult& cullResult, RenderFramesData& framesData, DepthData& depthData, RenderGraphResource rtMain)
	//{
	//	struct OpaqueSceneData
	//	{
	//		GPU::ViewPort mViewport;
	//		CullingResult mCullResult;
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

	void RenderPipeline3D::Setup(RenderGraph& graph, const Viewport& viewport, FrameCB& frameCB, const CullingResult& cullResult)
	{
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