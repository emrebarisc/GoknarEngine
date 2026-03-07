#ifndef __ANIMATIONNODE_H__
#define __ANIMATIONNODE_H__

#include "Core.h"

#include <memory>

struct AnimationNodeTransition;

struct GOKNAR_API AnimationNode
{
	AnimationNode() = default;
	~AnimationNode() = default;

    std::string animationName{ "" };

	std::vector<std::shared_ptr<AnimationNodeTransition>> outboundConnections;

    bool loop{ true };
};

#endif