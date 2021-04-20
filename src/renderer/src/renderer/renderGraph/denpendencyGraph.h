#pragma once

#include "renderer\definitions.h"
#include "core\container\map.h"

namespace Cjing3D
{
	class RenderPass;

	class DenpendencyGraph
	{
	public:
		using NodeID = U32;
		const static NodeID INVALID_NODE = -1;

		class Edge
		{
		public:
			const NodeID mFromNode;
			const NodeID mToNode;

			Edge(const Edge& rhs) = delete;
			void operator=(const Edge& rhs) = delete;
		};

		class Node
		{
		public:
			Node(DenpendencyGraph& graph);
			virtual ~Node() = default;

			NodeID GetID()const {  
				return mID;
			}
			I32 GetRefCount()const {
				return mRef;
			}

		private:
			Node(const Node& rhs) = delete;
			void operator=(const Node& rhs) = delete;

			DenpendencyGraph& mGraph;
			const NodeID mID;
			I32 mRef = 0;
		};

		using EdgeArray = DynamicArray<Edge*>;
		using NodeArray = DynamicArray<Node*>;

	public:
		DenpendencyGraph();
		~DenpendencyGraph();

		void Add(RenderPass& renderPass, I32 ref = 0);
		void SetRef(RenderPass& renderPass, I32 ref);
		void Cull();
		void Clear();

		EdgeArray& GetEdges() { return mEdges; }
		NodeArray& GetNodes() { return mNodes; }
		const EdgeArray& GetEdges()const { return mEdges; }
		const NodeArray& GetNodes()const { return mNodes; }

	private:
		void AddImpl(Node* val, I32 id);
		I32 GenerateID()const;

	private:
		DenpendencyGraph(const DenpendencyGraph& rhs) = delete;
		void operator=(const DenpendencyGraph& rhs) = delete;

		EdgeArray mEdges;
		NodeArray mNodes;
	};
}