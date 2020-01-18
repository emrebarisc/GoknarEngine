#include "pch.h"

#include "MeshComponent.h"

#include "Goknar/Engine.h"
#include "Goknar/Model/MeshInstance.h"

MeshComponent::MeshComponent(ObjectBase* parent) : Component(parent), meshInstance_(new MeshInstance(this))
{
}

MeshComponent::~MeshComponent()
{

}

void MeshComponent::SetMesh(Mesh* mesh)
{
	meshInstance_->SetMesh(mesh);
}

void MeshComponent::WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix)
{
	meshInstance_->SetWorldTransformationMatrix(worldTransformationMatrix);
}

void MeshComponent::UpdateRelativeTransformationMatrix()
{
	Component::UpdateRelativeTransformationMatrix();

	meshInstance_->SetRelativeTransformationMatrix(relativeTransformationMatrix_);
}