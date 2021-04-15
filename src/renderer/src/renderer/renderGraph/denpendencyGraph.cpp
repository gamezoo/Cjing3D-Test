#include "denpendencyGraph.h"

namespace Cjing3D
{
	DenpendencyGraph::DenpendencyGraph()
	{
	}

	DenpendencyGraph::~DenpendencyGraph()
	{
	}

	void DenpendencyGraph::Add(RenderPass& renderPass, I32 ref)
	{
		RenderPassNode node;
		node.mRenderPass = &renderPass;
		node.mRef = ref;
		mGraph.Add(node);
	}

	void DenpendencyGraph::SetRef(RenderPass& renderPass, I32 ref)
	{
	}

	void DenpendencyGraph::Cull()
	{
	}

	void DenpendencyGraph::Clear()
	{
	}
}