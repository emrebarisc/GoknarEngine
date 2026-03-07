#ifndef __ANIMATIONSTATE_H__
#define __ANIMATIONSTATE_H__

#include <memory>

#include "Core.h"

struct AnimationNodeTransition;

struct AnimationNode;

struct GOKNAR_API AnimationState
{
	AnimationState() = default;
	~AnimationState() = default;

	std::shared_ptr<AnimationNode> currentNode{ nullptr };
};

#endif