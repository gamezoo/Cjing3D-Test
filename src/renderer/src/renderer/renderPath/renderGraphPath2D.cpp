#include "renderGraphPath2D.h"
#include "renderer\renderImage.h"

namespace Cjing3D
{
	const char* RenderGraphPath2D::RT_MAIN_NAME = "rtMain2D";

	RenderGraphPath2D::RenderGraphPath2D()
	{
	}

	RenderGraphPath2D::~RenderGraphPath2D()
	{
	}

	void RenderGraphPath2D::ResizeBuffers()
	{
		auto resolution = GPU::GetResolution();

		GPU::TextureDesc desc;
		desc.mWidth = resolution[0];
		desc.mHeight = resolution[1];
		desc.mFormat = GPU::GetBackBufferFormat();
		desc.mBindFlags = GPU::BIND_RENDER_TARGET | GPU::BIND_SHADER_RESOURCE;
		GPU::ResHandle rtMain = GPU::CreateTexture(&desc, nullptr, "rtMain2D");
		mRtMain.SetTexture(rtMain, desc);
	}

	void RenderGraphPath2D::Start()
	{
		RenderPath::Start();

		if (!mResolutionChangedHandle.IsConnected())
		{
			ResizeBuffers();
			mResolutionChangedHandle = EventSystem::Register(EVENT_RESOLUTION_CHANGE,
				[this](const VariantArray& variants) {
					ResizeBuffers();
				});
		}
	}

	void RenderGraphPath2D::Stop()
	{
		mResolutionChangedHandle.Disconnect();
		mRenderPipeline2D.Clear();
	}

	void RenderGraphPath2D::Update(F32 dt)
	{
		RenderGraphPath::Update(dt);
	}

	void RenderGraphPath2D::UpdatePipelines()
	{
		auto rtMainRes = mMainGraph.ImportTexture(RT_MAIN_NAME, mRtMain.GetHandle(), &mRtMain.GetDesc());
		mRenderPipeline2D.SetResource("rtMain", rtMainRes);
		mRenderPipeline2D.Setup(mMainGraph);

		AddFinalResource(rtMainRes);

		RenderGraphPath::UpdatePipelines();
	}

	void RenderGraphPath2D::Compose(GPU::CommandList& cmd)
	{
		if (mRtMain.GetHandle())
		{
			ImageParams params;
			params.mBlendFlag  = BLENDMODE_PREMULTIPLIED;
			params.EnableFullScreen();

			RenderImage::Draw(mRtMain.GetHandle(), params, cmd);
		}

		RenderPath::Compose(cmd);
	}
}