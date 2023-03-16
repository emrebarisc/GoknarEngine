#include "pch.h"

#include "AnimatedSpriteComponent.h"

#include "Goknar/Model/2D/AnimatedSpriteMesh.h"
#include "Goknar/Model/DynamicMeshInstance.h"
#include "Goknar/Engine.h"
#include "Goknar/Log.h"

AnimatedSpriteComponent::AnimatedSpriteComponent(ObjectBase* parent) : 
	MeshComponent(parent, new DynamicMeshInstance(this)),
	animatedSpriteMesh_(nullptr)
{
}

AnimatedSpriteComponent::~AnimatedSpriteComponent()
{

}

void AnimatedSpriteComponent::SetMesh(MeshUnit* animatedSpriteMesh)
{
	animatedSpriteMesh_ = dynamic_cast<AnimatedSpriteMesh*>(animatedSpriteMesh);
	GOKNAR_ASSERT(animatedSpriteMesh_, "AnimatedSpriteComponent::SetMesh(Mesh*) assigned mesh is not a AnimatedSpriteMesh!");
	meshInstance_->SetMesh(animatedSpriteMesh);
}