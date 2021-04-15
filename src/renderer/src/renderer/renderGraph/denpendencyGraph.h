#pragma once

#include "renderer\definitions.h"
#include "core\container\graph.h"
#include "core\container\map.h"

namespace Cjing3D
{
	class DenpendencyGraph
	{
	public:
		DenpendencyGraph();
		~DenpendencyGraph();

		void Add(RenderPass& renderPass, I32 ref = 0);
		void SetRef(RenderPass& renderPass, I32 ref);
		void Cull();
		void Clear();

	private:
		DenpendencyGraph(const DenpendencyGraph& rhs) = delete;
		void operator=(const DenpendencyGraph& rhs) = delete;

		struct RenderPassNode
		{
			RenderPass* mRenderPass = nullptr;
			I32 mRef = 0;
		};
		Graph<RenderPassNode> mGraph;
		Map<RenderPass*, Graph<RenderPassNode>::NodeID>;
	};
}