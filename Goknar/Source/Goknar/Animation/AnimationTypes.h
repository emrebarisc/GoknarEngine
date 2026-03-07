#ifndef __ANIMATIONTYPES_H__
#define __ANIMATIONTYPES_H__

#include <variant>

#include "Goknar/Math/GoknarMath.h"

using AnimationVariable = std::variant<bool, int, float, Vector2, Vector2i, Vector3, Vector3i, Vector4>;

enum class CompareOp
{
	Equal,
	NotEqual,
	Greater,
	Less,
	GreaterOrEqual,
	LessOrEqual
};

#endif