#include "pch.h"

#include "SpriteComponent.h"

#include "Goknar/Model/DynamicMeshInstance.h"
#include "Goknar/Engine.h"
#include "Goknar/Model/2D/SpriteMesh.h"

SpriteComponent::SpriteComponent(ObjectBase* parent) : 
	MeshComponent(parent, new DynamicMeshInstance(this))
{
}

SpriteComponent::~SpriteComponent()
{

}

void SpriteComponent::SetMesh(Mesh* spriteMesh)
{
	meshInstance_->SetMesh(spriteMesh);
}

void SpriteComponent::WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix)
{
	meshInstance_->SetWorldTransformationMatrix(worldTransformationMatrix);
}

void SpriteComponent::UpdateRelativeTransformationMatrix()
{
	Component::UpdateRelativeTransformationMatrix();

	meshInstance_->SetRelativeTransformationMatrix(relativeTransformationMatrix_);
}