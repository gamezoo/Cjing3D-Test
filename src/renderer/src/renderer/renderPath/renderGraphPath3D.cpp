#include "renderGraphPath3D.h"
#include "gpu\gpu.h"
#include "renderer\renderImage.h"
#include "renderer\textureHelper.h"

namespace Cjing3D
{
	RenderGraphPath3D::RenderGraphPath3D()
	{
	}

	RenderGraphPath3D::~RenderGraphPath3D()
	{
	}

	void RenderGraphPath3D::ResizeBuffers(const U32x2& resolution)
	{
		// update viewport 
		mViewport.CreatePerspective((F32)resolution.x(), (F32)resolution.y(), mViewport.mNear, mViewport.mFar);

		// renderTargets
		{
			GPU::TextureDesc desc;
			desc.mWidth = resolution[0];
			desc.mHeight = resolution[1];
			desc.mFormat = GPU::FORMAT_R8G8B8A8_UNORM;
			desc.mBindFlags = GPU::BIND_RENDER_TARGET | GPU::BIND_SHADER_RESOURCE;			
			TextureHelper::CreateTexture(mRtMain, desc, nullptr, "rtMain3D");
		}
		// depthBuffers
		{
			GPU::TextureDesc desc;
			desc.mWidth = resolution[0];
			desc.mHeight = resolution[1];
			desc.mFormat = GPU::FORMAT_R32G8X24_TYPELESS;
			desc.mBindFlags = GPU::BIND_DEPTH_STENCIL | GPU::BIND_SHADER_RESOURCE;
			TextureHelper::CreateTexture(mDepthMain, desc, nullptr, "dbpMain");
		}

		RenderGraphPath2D::ResizeBuffers(resolution);
	}

	void RenderGraphPath3D::Start()
	{
		RenderGraphPath2D::Start();
	}

	void RenderGraphPath3D::Stop()
	{
		mRenderPipeline3D.Clear();
	}

	void RenderGraphPath3D::Update(F32 dt)
	{
		RenderGraphPath2D::Update(dt);

		// update visisbility
		I32 cullingFlag = CULLING_FLAG_ALL;
		mVisibility.mViewport = &mViewport;
		Renderer::UpdateViewCulling(mVisibility, mViewport, cullingFlag);

		// renderer update
		auto resolution = Renderer::GetInternalResolution();
		Renderer::UpdatePerFrameData(mVisibility, mFrameCB, dt, resolution);

		// update viewport
		mViewport.Update();
	}

	void RenderGraphPath3D::RenderPipelines(RenderGraph& renderGraph)
	{
		PROFILE_FUNCTION();

		// Setup render frame
		renderGraph.AddCallbackRenderPass("SetupRenderFrame",
			RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
			[&](RenderGraphResBuilder& builder) {

				return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {

				};
			});

		// Depth prepass 
		renderGraph.AddCallbackRenderPass("DepthPrepass",
			RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
			[&](RenderGraphResBuilder& builder) {

				return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {

				};
			});

		// Camera effects
		renderGraph.AddCallbackRenderPass("CameraEffect",
			RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
			[&](RenderGraphResBuilder& builder) {

				return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {

				};
			});

		// Shadow maps
		if (IsShadowEnable())
		{
			renderGraph.AddCallbackRenderPass("ShadowMaps",
				RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
				[&](RenderGraphResBuilder& builder) {

					return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {

					};
				});
		}

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

		// Render 2D
		RenderGraphPath2D::RenderPipelines(renderGraph);
	}

	void RenderGraphPath3D::ComposePipelines(GPU::CommandList& cmd)
	{
		cmd.EventBegin("Compose3D");
		if (mRtMain.GetHandle())
		{
			ImageParams params;
			params.mBlendFlag = BLENDMODE_OPAQUE;
			params.EnableFullScreen();

			RenderImage::Draw(mRtMain.GetHandle(), params, cmd);
		}
		cmd.EventEnd();

		RenderGraphPath2D::ComposePipelines(cmd);
	}

	void RenderGraphPath3D::AddFinalResources(RenderGraph& renderGraph)
	{
		RenderGraphPath2D::AddFinalResources(renderGraph);

		auto finalRes = renderGraph.GetBloackBoard().Get("rtMain3D");
		if (finalRes) {
			AddFinalResource(finalRes);
		}
	}
}