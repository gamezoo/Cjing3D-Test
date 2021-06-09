#include "renderGraphPath2D.h"
#include "renderer\renderImage.h"
#include "renderer\textureHelper.h"

namespace Cjing3D
{
	RenderGraphPath2D::RenderGraphPath2D()
	{
	}

	RenderGraphPath2D::~RenderGraphPath2D()
	{
	}

	void RenderGraphPath2D::ResizeBuffers(const U32x2& resolution)
	{
		GPU::TextureDesc desc;
		desc.mWidth = resolution[0];
		desc.mHeight = resolution[1];
		desc.mFormat = GPU::GetBackBufferFormat();
		desc.mBindFlags = GPU::BIND_RENDER_TARGET | GPU::BIND_SHADER_RESOURCE;
		TextureHelper::CreateTexture(mRtMain, desc, nullptr, "rtMain2D");
	}

	void RenderGraphPath2D::Start()
	{
		RenderPath::Start();
	}

	void RenderGraphPath2D::Stop()
	{
		RenderPath::Stop();
	}

	void RenderGraphPath2D::Update(F32 dt)
	{
		RenderGraphPath::Update(dt);
	}

	void RenderGraphPath2D::RenderPipelines(RenderGraph& renderGraph)
	{
		PROFILE_FUNCTION();

		auto rtMainRes = renderGraph.ImportTexture("rtMain2D", mRtMain.GetHandle(), mRtMain.GetDesc());
		renderGraph.AddCallbackRenderPass("Render2D",
			RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
			[&](RenderGraphResBuilder& builder) {

				// clear renderTarget
				RenderGraphAttachment attachment = RenderGraphAttachment::RenderTarget();
				attachment.mLoadOp = GPU::BindingFrameAttachment::LOAD_CLEAR;
				auto res = builder.AddRTV(rtMainRes, attachment);
				builder.GetBloackBoard().Put("rtMain2D", res);

				return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {
					
				};
			});
	}

	void RenderGraphPath2D::ComposePipelines(GPU::CommandList& cmd)
	{
		if (mRtMain.GetHandle())
		{
			ImageParams params;
			params.mBlendFlag  = BLENDMODE_PREMULTIPLIED;
			params.EnableFullScreen();

			RenderImage::Draw(mRtMain.GetHandle(), params, cmd);
		}
	}

	void RenderGraphPath2D::AddFinalResources(RenderGraph& renderGraph)
	{
		auto finalRes = renderGraph.GetBloackBoard().Get("rtMain2D");
		if (finalRes) {
			AddFinalResource(finalRes);
		}
	}
}