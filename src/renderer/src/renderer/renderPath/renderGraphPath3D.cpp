#include "renderGraphPath3D.h"
#include "gpu\gpu.h"
#include "renderer\renderImage.h"

namespace Cjing3D
{
	RenderGraphPath3D::RenderGraphPath3D()
	{
	}

	RenderGraphPath3D::~RenderGraphPath3D()
	{
	}

	void RenderGraphPath3D::ResizeBuffers()
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
			GPU::ResHandle rtMain = GPU::CreateTexture(&desc, nullptr, "rtMain");
			mRtMain.SetTexture(rtMain, desc);
		}
		// depthBuffers
		{
			GPU::TextureDesc desc;
			desc.mWidth = resolution[0];
			desc.mHeight = resolution[1];
			desc.mFormat = GPU::FORMAT_R32G8X24_TYPELESS;
			desc.mBindFlags = GPU::BIND_DEPTH_STENCIL | GPU::BIND_SHADER_RESOURCE;
			GPU::ResHandle dpbMain = GPU::CreateTexture(&desc, nullptr, "dpbMain");
			mDpbMain.SetTexture(dpbMain, desc);
		}

	}

	void RenderGraphPath3D::Start()
	{
		RenderGraphPath2D::Start();

		if (!mResolutionChangedHandle.IsConnected())
		{
			ResizeBuffers();
			mResolutionChangedHandle = EventSystem::Register(EVENT_RESOLUTION_CHANGE,
				[this](const VariantArray& variants) {
					ResizeBuffers();
			});
		}
	}

	void RenderGraphPath3D::Stop()
	{
		mResolutionChangedHandle.Disconnect();
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
		Renderer::Update(mVisibility, mFrameCB, dt);

		// update viewport
		mViewport.Update();
	}

	void RenderGraphPath3D::UpdatePipelines()
	{
		auto rtMainRes = mMainGraph.ImportTexture("rtMain3D", mRtMain.GetHandle(), &mRtMain.GetDesc());
		auto dbMainRes = mMainGraph.ImportTexture("dbMain", mDpbMain.GetHandle(), &mDpbMain.GetDesc());

		mRenderPipeline3D.SetResource("rtMain", rtMainRes);
		mRenderPipeline3D.SetResource("dbMain", dbMainRes);

		//setup pipelines
		mRenderPipeline3D.Setup(mMainGraph, mViewport, mFrameCB, mVisibility);
		AddFinalResource(rtMainRes);
		RenderGraphPath2D::UpdatePipelines();
	}

	void RenderGraphPath3D::Compose(GPU::CommandList& cmd)
	{
		if (mRtMain.GetHandle())
		{
			ImageParams params;
			params.mBlendFlag = BLENDMODE_OPAQUE;
			params.EnableFullScreen();

			RenderImage::Draw(mRtMain.GetHandle(), params, cmd);
		}

		RenderGraphPath2D::Compose(cmd);
	}
}