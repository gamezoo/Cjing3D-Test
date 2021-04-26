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

	void RenderGraphPath::Render()
	{
		// clear render graph
		mMainGraph.Clear();

		// update pipelines
		UpdatePipelines(mMainGraph);

		// compile render graph
		mMainGraph.Compile();

		// execute render graph
		mMainGraph.Execute();

		RenderPath::Render();
	}
}