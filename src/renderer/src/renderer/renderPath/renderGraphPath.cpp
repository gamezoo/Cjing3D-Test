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
	}

	void RenderGraphPath::Render()
	{
		mMainGraph.Clear();
		mFinalResources.clear();

		UpdatePipelines();

		if (mMainGraph.GetPassCount() > 0 && mFinalResources.size() > 0)
		{
			if (!mMainGraph.ExecuteWithoutSubmit(Span(mFinalResources.data(), mFinalResources.size()))) {
				Logger::Warning("Render graph failed to executed");
			}
		}

		RenderPath::Render();
	}

	void RenderGraphPath::Compose(GPU::CommandList& cmd)
	{
	}
}