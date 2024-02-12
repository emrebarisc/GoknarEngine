#include "pch.h"

#include "Box.h"

Box::Box() : 
	min_(Vector3::ZeroVector),
	max_(Vector3::ZeroVector),
	size_(Vector3::ZeroVector)
{
}

Box::Box(const Vector3& min, const Vector3& max) :
	min_(min),
	max_(max),
	size_(max_ - min_)
{
}
