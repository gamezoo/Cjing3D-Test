#include "renderGraphPath.h"
#include "gpu\gpu.h"
#include "renderer\renderImage.h"

namespace Cjing3D
{
	RenderGraphPath::RenderGraphPath()
	{
	}

	RenderGraphPath::~RenderGraphPath()
	{
	}

	void RenderGraphPath::Clear()
	{
	}

	void RenderGraphPath::ResizeBuffers()
	{
		auto resolution = GPU::GetResolution();

		// update viewport 
		mViewport.CreatePerspective((F32)resolution.x(), (F32)resolution.y(), mViewport.mNear, mViewport.mFar);

		// renderTargets
		{
			GPU::TextureDesc desc;
			desc.mWidth = resolution[0];
			desc.mHeight = resolution[1];
			desc.mFormat = GPU::FORMAT_R8G8B8A8_UNORM;
			desc.mBindFlags = GPU::BIND_RENDER_TARGET | GPU::BIND_SHADER_RESOURCE;
			GPU::ResHandle rtMain = GPU::CreateTexture(&desc, nullptr, "renderTarget_Main");
			mRenderTargetMain.SetTexture(rtMain, desc);
		}
		// depthBuffers
		{
			GPU::TextureDesc desc;
			desc.mWidth = resolution[0];
			desc.mHeight = resolution[1];
			desc.mFormat = GPU::FORMAT_R32G8X24_TYPELESS;
			desc.mBindFlags = GPU::BIND_DEPTH_STENCIL | GPU::BIND_SHADER_RESOURCE;
			GPU::ResHandle dsMain = GPU::CreateTexture(&desc, nullptr, "depthBuffer_Main");
			mDepthBufferMain.SetTexture(dsMain, desc);
		}

	}

	void RenderGraphPath::Start()
	{
		RenderPath::Start();

		if (!mResolutionChangedHandle.IsConnected())
		{
			ResizeBuffers();
			mResolutionChangedHandle = EventSystem::Register(EVENT_RESOLUTION_CHANGE,
				[this](const VariantArray& variants) {
					Clear();
					ResizeBuffers();
			});
		}
	}

	void RenderGraphPath::Stop()
	{
		mResolutionChangedHandle.Disconnect();

		// clear graph
		mMainGraph.Clear();
		mMainPipeline.Clear();

		// clear rtvs
		mRenderTargetMain.Clear();
		mDepthBufferMain.Clear();
	}

	void RenderGraphPath::Update(F32 dt)
	{
		RenderPath::Update(dt);

		// update visisbility
		I32 cullingFlag = CULLING_FLAG_ALL;
		mVisibility.mViewport = &mViewport;
		Renderer::UpdateViewCulling(mVisibility, mViewport, cullingFlag);

		// renderer update
		Renderer::Update(mVisibility, mFrameCB, dt);

		// update viewport
		mViewport.Update();
	}

	void RenderGraphPath::Render()
	{
		mMainGraph.Clear();

		// setup resources	
		auto SetupResource = [&](Texture& texture, const char* name) {
			auto res = mMainGraph.ImportTexture(name, texture.GetHandle(), &texture.GetDesc());
			mMainPipeline.SetResource(name, res);
		};
		SetupResource(mRenderTargetMain, "rtMain");
		SetupResource(mDepthBufferMain, "dbMain");

		//setup pipelines
		//mMainPipeline.Setup(mMainGraph, mViewport, mFrameCB, mVisibility);

		//if (!mMainGraph.Execute(mMainPipeline.GetResource("rtMain"))) {
		//	Debug::Warning("Render graph failed to executed");
		//}
	}

	void RenderGraphPath::Compose(GPU::CommandList& cmd)
	{
		if (mRenderTargetMain.GetHandle())
		{
			ImageParams params;
			params.mBlendFlag = BLENDMODE_OPAQUE;
			params.EnableFullScreen();

			RenderImage::Draw(mRenderTargetMain.GetHandle(), params, cmd);
		}

		RenderPath::Compose(cmd);
	}
}