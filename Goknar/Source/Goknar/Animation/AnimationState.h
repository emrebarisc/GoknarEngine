#ifndef __ANIMATIONSTATE_H__
#define __ANIMATIONSTATE_H__

#include <memory>
#include <string>

#include "Core.h"

struct AnimationNode;
struct AnimationNodeTransition;

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

	std::string name{ "" };

private:
	std::shared_ptr<AnimationNode> entryNode_{ nullptr };
	std::shared_ptr<AnimationNode> currentNode_{ nullptr };
};

#endif