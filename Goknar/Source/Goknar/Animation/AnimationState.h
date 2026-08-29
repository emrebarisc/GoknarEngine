#ifndef __ANIMATIONSTATE_H__
#define __ANIMATIONSTATE_H__

#include <memory>
#include <string>

#include "Core.h"

struct AnimationNode;

template<typename T>
struct AnimationTransition;

struct GOKNAR_API AnimationState
{
	friend struct AnimationGraph;

	AnimationState() = default;
	~AnimationState() = default;

	const std::shared_ptr<AnimationNode>& GetEntryNode() const
	{
		return entryNode_;
	}

	void SetEntryNode(const std::shared_ptr<AnimationNode>& entryNode)
	{
		entryNode_ = entryNode;
	}

	void Reset()
	{
		currentNode_ = entryNode_;
	}

	void AddNode(const std::shared_ptr<AnimationNode>& node)
	{
		nodes_.push_back(node);
	}

	const std::vector<std::shared_ptr<AnimationNode>>& GetNodes() const
	{
		return nodes_;
	}

	std::vector<std::shared_ptr<AnimationTransition<AnimationState>>> outboundConnections;

	std::string name{ "" };

private:
	std::vector<std::shared_ptr<AnimationNode>> nodes_{};
	std::shared_ptr<AnimationNode> entryNode_{ nullptr };
	std::shared_ptr<AnimationNode> currentNode_{ nullptr };
};

#endif
