#include "pch.h"

#include "AnimatedSpriteComponent.h"

#include "Goknar/Model/DynamicMeshInstance.h"
#include "Goknar/Engine.h"
#include "Goknar/Model/2D/AnimatedSpriteMesh.h"

AnimatedSpriteComponent::AnimatedSpriteComponent(ObjectBase* parent) : 
	MeshComponent(parent, new DynamicMeshInstance(this))
{
}

AnimatedSpriteComponent::~AnimatedSpriteComponent()
{

}

void AnimatedSpriteComponent::SetMesh(Mesh* animatedSpriteMesh)
{
	meshInstance_->SetMesh(animatedSpriteMesh);
}

void AnimatedSpriteComponent::WorldTransformationMatrixIsUpdated(const Matrix& worldTransformationMatrix)
{
	meshInstance_->SetWorldTransformationMatrix(worldTransformationMatrix);
}

void AnimatedSpriteComponent::UpdateRelativeTransformationMatrix()
{
	Component::UpdateRelativeTransformationMatrix();

	meshInstance_->SetRelativeTransformationMatrix(relativeTransformationMatrix_);
}