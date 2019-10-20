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
	relativeTransformationMatrix_ = Matrix(1.f, 0.f, 0.f, pivotPoint_.x,
										   0.f, 1.f, 0.f, pivotPoint_.y,
										   0.f, 0.f, 1.f, pivotPoint_.z,
										   0.f, 0.f, 0.f, 1.f);

	relativeTransformationMatrix_ *= Matrix(relativeScaling_.x, 0.f, 0.f, 0.f,
											0.f, relativeScaling_.y, 0.f, 0.f,
											0.f, 0.f, relativeScaling_.z, 0.f,
											0.f, 0.f, 0.f, 1.f);

	relativeTransformationMatrix_ *= Matrix::GetRotationMatrix(relativeRotation_);

	relativeTransformationMatrix_ *= Matrix(1.f, 0.f, 0.f, relativeLocation_.x,
											0.f, 1.f, 0.f, relativeLocation_.y,
											0.f, 0.f, 1.f, relativeLocation_.z,
											0.f, 0.f, 0.f, 1.f);

	relativeTransformationMatrix_ *= Matrix(1.f, 0.f, 0.f, -pivotPoint_.x,
										    0.f, 1.f, 0.f, -pivotPoint_.y,
										    0.f, 0.f, 1.f, -pivotPoint_.z,
										    0.f, 0.f, 0.f, 1.f);
}

void Component::UpdateWorldTransformationMatrix()
{

	worldTransformationMatrix_ = Matrix(worldScaling_.x, 0.f, 0.f, 0.f,
										 0.f, worldScaling_.y, 0.f, 0.f,
										 0.f, 0.f, worldScaling_.z, 0.f,
										 0.f, 0.f, 0.f, 1.f);

	worldTransformationMatrix_ *= Matrix::GetRotationMatrix(worldRotation_);

	worldTransformationMatrix_ *= Matrix(1.f, 0.f, 0.f, worldLocation_.x,
										 0.f, 1.f, 0.f, worldLocation_.y,
										 0.f, 0.f, 1.f, worldLocation_.z,
										 0.f, 0.f, 0.f, 1.f);
}
