#include "pch.h"

#include "ObjectBase.h"

#include "Goknar/Components/Component.h"
#include "Goknar/Engine.h"

ObjectBase::ObjectBase() : 
	tickable_(false),
	worldTransformationMatrix_(Matrix::IdentityMatrix),
	worldPosition_(Vector3::ZeroVector),
	worldRotation_(Vector3::ZeroVector),
	worldScaling_(Vector3(1.f)),
	totalComponentCount_(0)
{
	engine->RegisterObject(this);
}

void ObjectBase::SetTickable(bool tickable)
{
	tickable_ = tickable;
	if (tickable_)
	{
		engine->AddToTickableObjects(this);
	}
}

void ObjectBase::SetWorldPosition(const Vector3& position)
{
	worldPosition_ = position;
	UpdateWorldTransformationMatrix();
}

void ObjectBase::SetWorldRotation(const Vector3& rotation)
{
	worldRotation_ = rotation;
	UpdateWorldTransformationMatrix();
}

void ObjectBase::SetWorldScaling(const Vector3& scaling)
{
	worldScaling_ = scaling;
	UpdateWorldTransformationMatrix();
}

void ObjectBase::UpdateWorldTransformationMatrix()
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

	for (int componentIndex = 0; componentIndex < totalComponentCount_; componentIndex++)
	{
		components_[componentIndex]->WorldTransformationMatrixIsUpdated(worldTransformationMatrix_);
	}
}
