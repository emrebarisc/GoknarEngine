#ifndef __ANIMATIONNODE_H__
#define __ANIMATIONNODE_H__

#include "Core.h"

#include <memory>

template<typename T>
struct AnimationTransition;

struct GOKNAR_API AnimationNode
{
	AnimationNode() = default;
	~AnimationNode() = default;

    std::string animationName{ "" };

	std::vector<std::shared_ptr<AnimationTransition<AnimationNode>>> outboundConnections;

    bool loop{ true };
};

#endif