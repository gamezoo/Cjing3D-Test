#include "renderGraphPath.h"
#include "renderer\renderer.h"

namespace Cjing3D
{
	RenderGraphPath::RenderGraphPath()
	{
	}

	RenderGraphPath::~RenderGraphPath()
	{
	}

	void RenderGraphPath::Start()
	{
		RenderPath::Start();
	}

	void RenderGraphPath::Update(F32 dt)
	{
		U32x2 resolution = Renderer::GetInternalResolution();
		if (mCurrentBufferSize != resolution) 
		{
			mCurrentBufferSize = resolution;
			ResizeBuffers(resolution);
		}
	}

	void RenderGraphPath::Stop()
	{
		RenderPath::Stop();
	}

	void RenderGraphPath::Render()
	{
		// update pipelines
		RenderPipelines(mMainGraph);

		RenderPath::Render();
	}

	void RenderGraphPath::Compose(GPU::ResHandle rtHandle, const GPU::TextureDesc& rtDesc)
	{
		// import back buffer
		RenderGraphResource outColor = mMainGraph.ImportTexture("BackBuffer", rtHandle, rtDesc);

		mMainGraph.AddCallbackRenderPass("Compose",
			RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
			[&](RenderGraphResBuilder& builder) {

				// set input from final resources from pipelines
				for (const auto& input : mFinalResources) {
					builder.ReadTexture(input);
				}

				// set backbuffer rtv
				RenderGraphAttachment attachment = RenderGraphAttachment::RenderTarget();
				attachment.mUseCustomClearColor = true;
				for (U32 i = 0; i < 4; i++) {
					attachment.mCustomClearColor[i] = rtDesc.mClearValue.mColor[i];
				}
				builder.AddRTV(outColor, attachment);

				return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {
					// bind viewport
					GPU::ViewPort viewport;
					viewport.mHeight = (F32)rtDesc.mWidth;
					viewport.mWidth  = (F32)rtDesc.mHeight;
					cmd.BindViewport(viewport);

					ComposePipelines(cmd);
				};
			});

		// compile render graph
		mMainGraph.SetFinalResource(outColor);
		mMainGraph.Compile();

		// execute render graph
		JobSystem::JobHandle jobHandle;
		mMainGraph.Execute(jobHandle);
		JobSystem::Wait(&jobHandle);

		// present
		GPU::Present();

		// clear render graph
		mMainGraph.Clear();
	}

	void RenderGraphPath::AddFinalResource(const RenderGraphResource& res)
	{
		mFinalResources.push(res);
	}
}