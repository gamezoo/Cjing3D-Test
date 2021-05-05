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
			TextureHelper::CreateTexture(mRtMain, desc, nullptr, "dbpMain");
		}
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

		//auto rtMainRes = renderGraph.ImportTexture("rtMain3D", mRtMain.GetHandle(), &mRtMain.GetDesc());
		//auto dbMainRes = renderGraph.ImportTexture("dbMain", mDpbMain.GetHandle(), &mDpbMain.GetDesc());

		//mRenderPipeline3D.SetResource("rtMain", rtMainRes);
		//mRenderPipeline3D.SetResource("dbMain", dbMainRes);

		//setup pipelines
		// mRenderPipeline3D.Setup(renderGraph, mViewport, mFrameCB, mVisibility);
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
}