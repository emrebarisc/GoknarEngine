#include "pch.h"

#include "Component.h"

int Component::lastComponentId_ = 0;

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

void Component::SetWorldPosition(const Vector3& position)
{
	worldPosition_ = position;
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

void Component::UpdateWorldTransformationMatrix()
{
	// Since OpenGL uses column-major matriced and Goknar does not
	// all matrix multiplications are done in reverse order
	worldTransformationMatrix_ = Matrix(1.f, 0.f, 0.f, worldPosition_.x,
										0.f, 1.f, 0.f, worldPosition_.y,
										0.f, 0.f, 1.f, worldPosition_.z,
										0.f, 0.f, 0.f, 1.f);

	worldTransformationMatrix_ *= Matrix(worldScaling_.x, 0.f, 0.f, 0.f,
										 0.f, worldScaling_.y, 0.f, 0.f,
										 0.f, 0.f, worldScaling_.z, 0.f,
										 0.f, 0.f, 0.f, 1.f);

	worldTransformationMatrix_ *= Matrix::GetRotationMatrix(worldRotation_);
}
