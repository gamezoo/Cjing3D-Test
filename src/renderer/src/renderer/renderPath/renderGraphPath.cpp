#include "renderGraphPath.h"
#include "gpu\gpu.h"

namespace Cjing3D
{
	RenderGraphPath::RenderGraphPath()
	{
	}

	RenderGraphPath::~RenderGraphPath()
	{
		Clear();
	}

	void RenderGraphPath::Clear()
	{
		mRTMain.Clear();
	}

	void RenderGraphPath::ResizeBuffers()
	{
		auto resolution = GPU::GetResolution();

		GPU::TextureDesc desc;
		desc.mWidth = resolution[0];
		desc.mHeight = resolution[1];
		desc.mFormat = GPU::FORMAT_R8G8B8A8_UNORM;
		desc.mBindFlags = GPU::BIND_RENDER_TARGET | GPU::BIND_SHADER_RESOURCE;
		GPU::ResHandle rtMain = GPU::CreateTexture(&desc, nullptr, "rtMain");
		mRTMain.SetTexture(rtMain, desc);
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

		Clear();

		mRenderGraph.Clear();
		mMainPipeline.Clear();
	}

	void RenderGraphPath::Update(F32 dt)
	{
		mRenderGraph.Clear();

		// setup resources	
		auto resRTMain = mRenderGraph.ImportTexture("rtMain", mRTMain.GetHandle(), &mRTMain.GetDesc());
		mMainPipeline.SetResource("rtMain", resRTMain);

		// setup pipelines
		mMainPipeline.Setup(mRenderGraph);
	}

	void RenderGraphPath::FixedUpdate()
	{
	}

	void RenderGraphPath::Render()
	{
		if (!mRenderGraph.Execute(mMainPipeline.GetResource("rtMain"))) {
			Debug::Warning("Render graph failed to executed");
		}
	}

	void RenderGraphPath::Compose()
	{
	}
}