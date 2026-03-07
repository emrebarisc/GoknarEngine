#ifndef __ANIMATIONCONDITION_H__
#define __ANIMATIONCONDITION_H__

#include <string>
#include <variant>

#include "Goknar/Core.h"

#include "Goknar/Animation/AnimationTypes.h"
#include "Goknar/Animation/AnimationGraph.h"

struct GOKNAR_API AnimationCondition
{
	std::string variableName;
	CompareOp operation;
	AnimationVariable targetValue;

	inline bool Evaluate(const AnimationGraph* graph) const
	{
		const AnimationVariable* graphVariable = graph->GetVariable(variableName);

		if (!graphVariable || graphVariable->index() != targetValue.index())
		{
			return false;
		}

		return std::visit(
			[this](const auto& gVal, const auto& tVal) -> bool
			{
				using Type1 = std::decay_t<decltype(gVal)>;
				using Type2 = std::decay_t<decltype(tVal)>;

				if constexpr (std::is_same_v<Type1, Type2>)
				{
					switch (operation)
					{
					case CompareOp::Equal:
						return gVal == tVal;
						break;
					case CompareOp::NotEqual:
						return gVal != tVal;
						break;
					case CompareOp::Greater:
						return gVal > tVal;
						break;
					case CompareOp::Less:
						return gVal < tVal;
						break;
					case CompareOp::GreaterOrEqual:
						return gVal >= tVal;
						break;
					case CompareOp::LessOrEqual:
						return gVal <= tVal;
						break;
					default:
						return false;
					}
				}
				else
				{
					return false;
				}
			}, *graphVariable, targetValue);
	}
};

#endif