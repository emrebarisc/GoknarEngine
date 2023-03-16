#include "pch.h"

#include "SpriteComponent.h"

#include "Goknar/Model/DynamicMeshInstance.h"
#include "Goknar/Engine.h"
#include "Goknar/Model/2D/SpriteMesh.h"
#include "Goknar/Log.h"

SpriteComponent::SpriteComponent(ObjectBase* parent) : 
	MeshComponent(parent, new DynamicMeshInstance(this))
{
}

SpriteComponent::~SpriteComponent()
{

}

void SpriteComponent::SetMesh(MeshUnit* spriteMesh)
{
	spriteMesh_ = dynamic_cast<SpriteMesh*>(spriteMesh);
	GOKNAR_ASSERT(spriteMesh_, "SpriteComponent::SetMesh(Mesh*) assigned mesh is not a SpriteMesh!");
	meshInstance_->SetMesh(spriteMesh);
}