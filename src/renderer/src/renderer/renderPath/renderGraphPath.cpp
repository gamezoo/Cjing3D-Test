#include "renderGraphPath.h"

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

	void RenderGraphPath::Stop()
	{
		RenderPath::Stop();
	}

	void RenderGraphPath::AddFinalResource(RenderGraphResource res)
	{
		mFinalResources.push(res);
	}

	void RenderGraphPath::Update(F32 dt)
	{
		mMainGraph.Clear();
		mFinalResources.clear();

		UpdatePipelines();
	}

	void RenderGraphPath::Render()
	{
		if (mMainGraph.GetPassCount() > 0 && mFinalResources.size() > 0)
		{
			if (!mMainGraph.ExecuteWithoutSubmit(Span(mFinalResources.data(), mFinalResources.size()))) {
				Debug::Warning("Render graph failed to executed");
			}
		}

		RenderPath::Render();
	}

	void RenderGraphPath::Compose(GPU::CommandList& cmd)
	{
	}
}