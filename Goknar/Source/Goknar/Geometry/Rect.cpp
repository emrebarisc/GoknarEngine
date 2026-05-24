#include "pch.h"

#include "Rect.h"

Rect::Rect() : 
	min_(Vector3::ZeroVector),
	max_(Vector3::ZeroVector),
	depth_(0.f),
	width_(0.f),
	height_(0.f)
{
}

Rect::Rect(const Vector3& min, const Vector3& max) :
	min_(min),
	max_(max),
	depth_(abs(max.x - min.x)),
	width_(abs(max.y - min.y)),
	height_(abs(max.z - min.z))
{
}
