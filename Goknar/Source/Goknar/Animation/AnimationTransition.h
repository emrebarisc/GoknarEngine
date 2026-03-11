#ifndef __STATECONNECTION_H__
#define __STATECONNECTION_H__

#include <memory>
#include <vector>

#include "Core.h"

#include "Goknar/Animation/AnimationCondition.h"

struct AnimationNode;
struct AnimationGraph;

template<typename T>
struct GOKNAR_API AnimationTransition
{
	AnimationTransition() = default;
	~AnimationTransition() = default;

	bool ShouldTransit(const AnimationGraph* graph, bool isCurrentStateFinished) const
	{
		if (transitWhenAnimationDone && isCurrentStateFinished)
		{
			return true;
		}

		if (!conditions.empty())
		{
			for (const auto& condition : conditions)
			{
				if (!condition.Evaluate(graph))
				{
					return false;
				}
			}

			return true;
		}

		return false;
	}

	std::shared_ptr<T> target{ nullptr };

	bool transitWhenAnimationDone{ false };

	std::vector<AnimationCondition> conditions;
};

#endif