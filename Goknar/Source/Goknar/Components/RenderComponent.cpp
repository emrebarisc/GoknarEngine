#include "pch.h"

#include "RenderComponent.h"
#include "Goknar/ObjectBase.h"

RenderComponent::RenderComponent(ObjectBase* parent) :
	Component(parent),
	relativeTransformationMatrix_(Matrix::IdentityMatrix),
	relativePosition_(Vector3::ZeroVector),
	relativeRotation_(Vector3::ZeroVector),
	relativeScaling_(Vector3(1.f)),
	pivotPoint_(Vector3::ZeroVector)
{
}

void RenderComponent::SetPivotPoint(const Vector3& pivotPoint)
{
	pivotPoint_ = pivotPoint;
	UpdateRelativeTransformationMatrix();
}

void RenderComponent::SetRelativePosition(const Vector3& position)
{
	relativePosition_ = position;
	UpdateRelativeTransformationMatrix();
}

void RenderComponent::SetRelativeRotation(const Vector3& rotation)
{
	relativeRotation_ = rotation;
	UpdateRelativeTransformationMatrix();
}

void RenderComponent::SetRelativeScaling(const Vector3& scaling)
{
	relativeScaling_ = scaling;
	UpdateRelativeTransformationMatrix();
}