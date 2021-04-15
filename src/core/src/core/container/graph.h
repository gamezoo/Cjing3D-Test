#pragma once

#include "core\common\definitions.h"
#include "core\memory\memory.h"
#include "core\container\dynamicArray.h"

namespace Cjing3D
{
	template<typename T>
	class Graph
	{
	public:
		using NodeID = U32;

		struct Edge
		{
			NodeID mFromNode;
			NodeID mToNode;
		};

		struct Node
		{
			const NodeID mID;
			T mValue;
		};

		using EdgeArray = DynamicArray<Edge>;
		using NodeArray = DynamicArray<Node>;

	public:
		Graph() = default;
		Graph(const Graph& rhs)
		{

		}
		Graph(Graph&& rhs)
		{
			swap(rhs);
		}
		~Graph()
		{
		}

		void operator= (const Graph& rhs)
		{
		}
		void operator= (Graph&& rhs)
		{
		}

		void Add(const T& val)
		{
			AddImpl(val, GenerateID());
		}

	private:
		void AddImpl(const T& val, I32 id)
		{
		}

		I32 GenerateID()const
		{
			return 0;
		}

	private:
		EdgeArray mEdges;
		NodeArray mNodes;
		ContainerAllocator mAllocator;
	};
}