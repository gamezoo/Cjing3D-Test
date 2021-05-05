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
		mRenderPipeline2D.Clear();
	}

	void RenderGraphPath2D::Update(F32 dt)
	{
		RenderGraphPath::Update(dt);
	}

	void RenderGraphPath2D::RenderPipelines(RenderGraph& renderGraph)
	{
		PROFILE_FUNCTION();

		//auto rtMainRes = renderGraph.ImportTexture(RT_MAIN_NAME, mRtMain.GetHandle(), &mRtMain.GetDesc());
		//mRenderPipeline2D.SetResource("rtMain", rtMainRes);
		// mRenderPipeline2D.Setup(renderGraph);

		//AddFinalResource(rtMainRes);
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
}