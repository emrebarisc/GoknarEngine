#include "pch.h"

#include "Box.h"

Box::Box() : 
	min_(Vector3::ZeroVector),
	max_(Vector3::ZeroVector),
	width_(0),
	depth_(0),
	height_(0)
{
}

Box::Box(const Vector3& min, const Vector3& max) :
	min_(min),
	max_(max),
	width_(abs(max.x - min.x)),
	depth_(abs(max.y - min.y)),
	height_(abs(max.z - min.z))
{
}
