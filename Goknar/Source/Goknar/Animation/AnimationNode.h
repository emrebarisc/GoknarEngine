#ifndef __ANIMATIONNODE_H__
#define __ANIMATIONNODE_H__

#include "Core.h"

#include <memory>
#include <string>
#include <vector>

template<typename T>
struct AnimationTransition;

enum class AnimationNodeType
{
	Clip,
	BlendSpace1D,
	BlendSpace2D
};

struct GOKNAR_API BlendSpace1DPoint
{
	float value{ 0.f };
	std::string animationName{};
};

struct GOKNAR_API BlendSpace2DPoint
{
	float x{ 0.f };
	float y{ 0.f };
	std::string animationName{};
};

struct GOKNAR_API AnimationNode
{
	AnimationNode() = default;
	~AnimationNode() = default;

	AnimationNodeType type{ AnimationNodeType::Clip };

    std::string animationName{ "" };
	std::string parameterName{ "" };
	std::string parameterXName{ "" };
	std::string parameterYName{ "" };
	std::string syncGroup{ "" };

	float playRate{ 1.f };
	float parameterSmoothingSpeed{ 0.f };

	std::vector<std::shared_ptr<AnimationTransition<AnimationNode>>> outboundConnections;
	std::vector<BlendSpace1DPoint> blendSpace1DPoints{};
	std::vector<BlendSpace2DPoint> blendSpace2DPoints{};

    bool loop{ true };
};

#endif
