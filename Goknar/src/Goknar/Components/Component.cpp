#include "pch.h"

#include "Component.h"
#include "Goknar/ObjectBase.h"

Component::Component(ObjectBase* parent) :
	parent_(parent),
	relativeTransformationMatrix_(Matrix::IdentityMatrix),
	relativePosition_(Vector3::ZeroVector),
	relativeRotation_(Vector3::ZeroVector),
	relativeScaling_(Vector3(1.f)),
	pivotPoint_(Vector3::ZeroVector)
{
	parent->AddComponent(this);
}

void Component::SetPivotPoint(const Vector3& pivotPoint)
{
	pivotPoint_ = pivotPoint;
	UpdateRelativeTransformationMatrix();
}

void Component::SetRelativePosition(const Vector3& position)
{
	relativePosition_ = position;
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