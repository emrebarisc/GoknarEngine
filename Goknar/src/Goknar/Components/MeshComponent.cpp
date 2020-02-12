#include "pch.h"

#include "MeshComponent.h"

#include "Goknar/Engine.h"
#include "Goknar/Model/MeshInstance.h"
#include "Goknar/ObjectBase.h"

MeshComponent::MeshComponent(ObjectBase* parent) : Component(parent), meshInstance_(new MeshInstance(this))
{
	WorldTransformationMatrixIsUpdated(parent->GetWorldTransformationMatrix());
}

MeshComponent::~MeshComponent()
{

}

void MeshComponent::Destroy()
{
	meshInstance_->RemoveFromRenderer();
	delete meshInstance_;
}

void MeshComponent::SetMesh(Mesh* mesh)
{
	meshInstance_->SetMesh(mesh);
}

void MeshComponent::WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix)
{
	meshInstance_->SetWorldTransformationMatrix(worldTransformationMatrix);
}

void MeshComponent::SetIsRendered(bool isRendered)
{
	isRendered_ = isRendered;
	meshInstance_->SetIsRendered(isRendered_);
}

void MeshComponent::UpdateRelativeTransformationMatrix()
{
	Component::UpdateRelativeTransformationMatrix();

	meshInstance_->SetRelativeTransformationMatrix(relativeTransformationMatrix_);
}