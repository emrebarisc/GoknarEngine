#include "pch.h"

#include "Model.h"

int Model::lastComponentId_ = 0;

void Model::SetRelativePosition(const Vector3& position)
{
	relativePosition_ = position;
	UpdateRelativeTransformationMatrix();
}

void Model::SetRelativeRotation(const Vector3& rotation)
{
	relativeRotation_ = rotation;
	UpdateRelativeTransformationMatrix();
}

void Model::SetRelativeScaling(const Vector3& scaling)
{
	relativeScaling_ = scaling;
	UpdateRelativeTransformationMatrix();
}

void Model::SetWorldPosition(const Vector3& position)
{
	worldPosition_ = position;
	UpdateWorldTransformationMatrix();
}

void Model::SetWorldRotation(const Vector3& rotation)
{
	worldRotation_ = rotation;
	UpdateWorldTransformationMatrix();
}

void Model::SetWorldScaling(const Vector3& scaling)
{
	worldScaling_ = scaling;
	UpdateWorldTransformationMatrix();
}

void Model::UpdateRelativeTransformationMatrix()
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

void Model::UpdateWorldTransformationMatrix()
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
