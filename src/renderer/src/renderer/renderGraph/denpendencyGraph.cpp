#include "denpendencyGraph.h"
#include "renderer\renderGraph\renderPass.h"

namespace Cjing3D
{
	DenpendencyGraph::Node::Node(DenpendencyGraph& graph) :
		mGraph(graph),
		mID(graph.GenerateID())
	{
	}

	DenpendencyGraph::DenpendencyGraph()
	{
	}

	DenpendencyGraph::~DenpendencyGraph()
	{
	}

	void DenpendencyGraph::Add(RenderPass& renderPass, I32 ref)
	{
	}

	void DenpendencyGraph::SetRef(RenderPass& renderPass, I32 ref)
	{
	}

	void DenpendencyGraph::Cull()
	{
		//auto& edges = mGraph.GetEdges();
		//auto& nodes = mGraph.GetNodes();

		//// ����Edge����������ڵ��Ref
		//for (const auto& edge : edges)
		//{
		//	if (auto node = mGraph.GetNode(edge.mFromNode)) {
		//		node->mValue.mRef++;
		//	}
		//}

		//// ��ȡ����Ref==0�Ľڵ�
		//DynamicArray<GraphType::Node*> removedNodes;
		//for (auto& node : nodes)
		//{
		//	if (node.mValue.mRef <= 0) {
		//		removedNodes.push(&node);
		//	}
		//}

		//// �Ƴ�Ref==0�Ľڵ㣬ͬʱ����Incomming�ڵ������ֵ�������Ƴ���Ч�ķ�֧
		//DynamicArray<GraphType::Edge> edges;
		//while (removedNodes.empty())
		//{
		//	GraphType::Node* node = removedNodes.back();
		//	removedNodes.pop();
		//	mGraph.GetIncommingEdge(node->mID, edges);
		//	for (const auto& edge : edges)
		//	{
		//		if (auto node = mGraph.GetNode(edge.mFromNode)) 
		//		{
		//			node->mValue.mRef--;
		//			if (node->mValue.mRef <= 0) {
		//				removedNodes.push(node);
		//			}
		//		}
		//	}
		//	edges.clear();
		//}
	}

	void DenpendencyGraph::Clear()
	{
		mNodes.clear();
		mEdges.clear();
	}

	void DenpendencyGraph::AddImpl(Node* val, I32 id)
	{
	}

	I32 DenpendencyGraph::GenerateID()const
	{
		return 0;
	}
}