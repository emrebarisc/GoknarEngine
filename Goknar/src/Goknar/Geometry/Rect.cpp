//#include "pch.h"

#include "Rect.h"

Rect::Rect() : 
	min_(Vector2::ZeroVector),
	max_(Vector2::ZeroVector),
	width_(0),
	height_(0)
{
}

Rect::Rect(const Vector2& min, const Vector2& max) :
	min_(min),
	max_(max),
	width_(abs(max.x - min.x)),
	height_(abs(max.y - min.y))
{
}
