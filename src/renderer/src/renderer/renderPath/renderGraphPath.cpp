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

	void RenderGraphPath::Render()
	{
		mFinalResources.clear();
		
		// update pipelines
		UpdatePipelines(mMainGraph);

		// compile render graph
		mMainGraph.Compile();

		// execute render graph
		mMainGraph.Execute();

		// clear render graph
		mMainGraph.Clear();

		RenderPath::Render();
	}
}