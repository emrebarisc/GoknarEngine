#include "NavigationTree.h"

#include <algorithm>

NavigationTree::NavigationTree()
{

}

NavigationTree::~NavigationTree()
{
	Clear();
}

NavigationNode* NavigationTree::AddNode(const Area& area)
{
	std::unique_ptr<NavigationNode> node = std::make_unique<NavigationNode>();
	node->id = nextNodeId_++;
	node->area = area;

	NavigationNode* nodeRaw = node.get();
	nodes_.push_back(std::move(node));

	if (!root)
	{
		root = nodeRaw;
	}

	return nodeRaw;
}

void NavigationTree::RemoveNode(NavigationNode* node)
{
	if (!node)
	{
		return;
	}

	for (std::unique_ptr<NavigationNode>& otherNode : nodes_)
	{
		if (!otherNode)
		{
			continue;
		}

		otherNode->neighbours.erase(
			std::remove(otherNode->neighbours.begin(), otherNode->neighbours.end(), node),
			otherNode->neighbours.end());
	}

	nodes_.erase(
		std::remove_if(
			nodes_.begin(),
			nodes_.end(),
			[node](const std::unique_ptr<NavigationNode>& candidate)
			{
				return candidate.get() == node;
			}),
		nodes_.end());

	if (root == node)
	{
		root = nodes_.empty() ? nullptr : nodes_.front().get();
	}
}

void NavigationTree::Clear()
{
	root = nullptr;
	nodes_.clear();
	nextNodeId_ = 0;
}

NavigationNode* NavigationTree::GetNodeById(int id) const
{
	for (const std::unique_ptr<NavigationNode>& node : nodes_)
	{
		if (node && node->id == id)
		{
			return node.get();
		}
	}

	return nullptr;
}

int NavigationTree::GetNodeIndex(const NavigationNode* node) const
{
	for (int nodeIndex = 0; nodeIndex < (int)nodes_.size(); ++nodeIndex)
	{
		if (nodes_[nodeIndex].get() == node)
		{
			return nodeIndex;
		}
	}

	return -1;
}

void NavigationTree::EnsureNextNodeId(int nextNodeId)
{
	nextNodeId_ = GoknarMath::Max(nextNodeId_, nextNodeId);
}
