#include "pch.h"

#include "Component.h"

int Component::lastComponentId_ = 0;

void Component::SetRelativeLocation(const Vector3& location)
{
	relativeLocation_ = location;
	UpdateRelativeTransformationMatrix();
}

void Component::SetRelativeRotation(const Vector3& rotation)
{
	relativeRotation_ = rotation;
	UpdateRelativeTransformationMatrix();
}

void Component::SetRelativeScaling(const Vector3& scaling)
{
	relativeScaling_ = scaling;
	UpdateRelativeTransformationMatrix();
}

void Component::SetWorldLocation(const Vector3& location)
{
	worldLocation_ = location;
	UpdateWorldTransformationMatrix();
}

void Component::SetWorldRotation(const Vector3& rotation)
{
	worldRotation_ = rotation;
	UpdateWorldTransformationMatrix();
}

void Component::SetWorldScaling(const Vector3& scaling)
{
	worldScaling_ = scaling;
	UpdateWorldTransformationMatrix();
}

void Component::UpdateRelativeTransformationMatrix()
{
	relativeTransformationMatrix_ = Matrix(relativeScaling_.x, 0.f, 0.f, relativeLocation_.x,
											0.f, relativeScaling_.y, 0.f, relativeLocation_.y,
											0.f, 0.f, relativeScaling_.z, relativeLocation_.z,
											0.f, 0.f, 0.f, 1.f);

	if (relativeRotation_.x != 0)
	{
		float cosTheta = cos(relativeRotation_.x);
		float sinTheta = sin(relativeRotation_.x);
		relativeTransformationMatrix_.m[5] *= cosTheta;
		relativeTransformationMatrix_.m[6] *= -sinTheta;
		relativeTransformationMatrix_.m[9] *= sinTheta;
		relativeTransformationMatrix_.m[10] *= cosTheta;
	}
	if (relativeRotation_.y != 0)
	{
		float cosTheta = cos(relativeRotation_.y);
		float sinTheta = sin(relativeRotation_.y);
		relativeTransformationMatrix_.m[0] *= cosTheta;
		relativeTransformationMatrix_.m[2] *= sinTheta;
		relativeTransformationMatrix_.m[8] *= -sinTheta;
		relativeTransformationMatrix_.m[10] *= cosTheta;
	}
	if (relativeRotation_.z != 0)
	{
		float cosTheta = cos(relativeRotation_.z);
		float sinTheta = sin(relativeRotation_.z);
		relativeTransformationMatrix_.m[0] *= cosTheta;
		relativeTransformationMatrix_.m[1] *= -sinTheta;
		relativeTransformationMatrix_.m[4] *= sinTheta;
		relativeTransformationMatrix_.m[5] *= cosTheta;
	}
}

void Component::UpdateWorldTransformationMatrix()
{	
	worldTransformationMatrix_ = Matrix(relativeScaling_.x, 0.f, 0.f, relativeLocation_.x,
										0.f, relativeScaling_.y, 0.f, relativeLocation_.y,
										0.f, 0.f, relativeScaling_.z, relativeLocation_.z,
										0.f, 0.f, 0.f, 1.f);

	if (worldRotation_.x != 0)
	{
		float cosTheta = cos(worldRotation_.x);
		float sinTheta = sin(worldRotation_.x);
		worldTransformationMatrix_.m[5] *= cosTheta;
		worldTransformationMatrix_.m[6] *= -sinTheta;
		worldTransformationMatrix_.m[9] *= sinTheta;
		worldTransformationMatrix_.m[10] *= cosTheta;
	}
	if (worldRotation_.y != 0)
	{
		float cosTheta = cos(worldRotation_.y);
		float sinTheta = sin(worldRotation_.y);
		worldTransformationMatrix_.m[0] *= cosTheta;
		worldTransformationMatrix_.m[2] *= sinTheta;
		worldTransformationMatrix_.m[8] *= -sinTheta;
		worldTransformationMatrix_.m[10] *= cosTheta;
	}
	if (worldRotation_.z != 0)
	{
		float cosTheta = cos(worldRotation_.z);
		float sinTheta = sin(worldRotation_.z);
		worldTransformationMatrix_.m[0] *= cosTheta;
		worldTransformationMatrix_.m[1] *= -sinTheta;
		worldTransformationMatrix_.m[4] *= sinTheta;
		worldTransformationMatrix_.m[5] *= cosTheta;
	}
}
