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