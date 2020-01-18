#include "pch.h"

#include "Component.h"
#include "Goknar/ObjectBase.h"

Component::Component(ObjectBase* parent) :
	parent_(parent),
	relativeTransformationMatrix_(Matrix::IdentityMatrix),
	relativePosition_(Vector3::ZeroVector),
	relativeRotation_(Vector3::ZeroVector),
	relativeScaling_(Vector3(1.f))
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

void Component::UpdateRelativeTransformationMatrix()
{
	// Since OpenGL uses column-major matriced and Goknar does not
	// all matrix multiplications are done in reverse order
	
	relativeTransformationMatrix_ = Matrix(1.f, 0.f, 0.f, pivotPoint_.x,
										   0.f, 1.f, 0.f, pivotPoint_.y,
										   0.f, 0.f, 1.f, pivotPoint_.z,
										   0.f, 0.f, 0.f, 1.f);

	relativeTransformationMatrix_ *= Matrix(1.f, 0.f, 0.f, relativePosition_.x,
											0.f, 1.f, 0.f, relativePosition_.y,
											0.f, 0.f, 1.f, relativePosition_.z,
											0.f, 0.f, 0.f, 1.f);

	relativeTransformationMatrix_ *= Matrix::GetRotationMatrix(relativeRotation_);

	relativeTransformationMatrix_ *= Matrix(relativeScaling_.x, 0.f, 0.f, 0.f,
											0.f, relativeScaling_.y, 0.f, 0.f,
											0.f, 0.f, relativeScaling_.z, 0.f,
											0.f, 0.f, 0.f, 1.f);

	relativeTransformationMatrix_ *= Matrix(1.f, 0.f, 0.f, -pivotPoint_.x,
										    0.f, 1.f, 0.f, -pivotPoint_.y,
										    0.f, 0.f, 1.f, -pivotPoint_.z,
										    0.f, 0.f, 0.f, 1.f);
}
