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

Box Box::Transform(const Matrix& transformationMatrix) const
{
	const Vector3& min = GetMin();
	const Vector3& max = GetMax();
	const Vector3 corners[8] =
	{
		Vector3(min.x, min.y, min.z),
		Vector3(max.x, min.y, min.z),
		Vector3(min.x, max.y, min.z),
		Vector3(max.x, max.y, min.z),
		Vector3(min.x, min.y, max.z),
		Vector3(max.x, min.y, max.z),
		Vector3(min.x, max.y, max.z),
		Vector3(max.x, max.y, max.z)
	};

	const Vector3 firstTransformedPoint = corners[0].TransformPosition(transformationMatrix);
	Box transformedAABB(firstTransformedPoint, firstTransformedPoint);
	for (int i = 1; i < 8; ++i)
	{
		transformedAABB.ExtendWRTPoint(corners[i].TransformPosition(transformationMatrix));
	}

	return transformedAABB;
}
