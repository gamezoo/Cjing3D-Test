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

		//// 根据Edge更新所有入节点的Ref
		//for (const auto& edge : edges)
		//{
		//	if (auto node = mGraph.GetNode(edge.mFromNode)) {
		//		node->mValue.mRef++;
		//	}
		//}

		//// 获取所有Ref==0的节点
		//DynamicArray<GraphType::Node*> removedNodes;
		//for (auto& node : nodes)
		//{
		//	if (node.mValue.mRef <= 0) {
		//		removedNodes.push(&node);
		//	}
		//}

		//// 移除Ref==0的节点，同时更新Incomming节点的引用值，意在移除无效的分支
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