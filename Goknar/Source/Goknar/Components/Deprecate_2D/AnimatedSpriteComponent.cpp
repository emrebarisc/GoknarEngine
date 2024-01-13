#include "pch.h"

#include "AnimatedSpriteComponent.h"

#include "Goknar/Model/2D/AnimatedSpriteMesh.h"
#include "Goknar/Model/DynamicMeshInstance.h"
#include "Goknar/Model/IMeshInstance.h"

AnimatedSpriteComponent::AnimatedSpriteComponent(Component* parent) :
	MeshComponent(parent, new DynamicMeshInstance(this))
{
}

AnimatedSpriteComponent::~AnimatedSpriteComponent()
{

}

void AnimatedSpriteComponent::SetMesh(DynamicMesh* dynamicMesh)
{
	meshInstance_->SetMesh(dynamicMesh);
}