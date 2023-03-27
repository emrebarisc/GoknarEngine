#include "pch.h"

#include "SkeletalMeshComponent.h"

#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Model/SkeletalMeshInstance.h"
#include "Goknar/Log.h"

SkeletalMeshComponent::SkeletalMeshComponent(Component* parent) : MeshComponent(parent)
{
}

SkeletalMeshComponent::~SkeletalMeshComponent()
{

}

void SkeletalMeshComponent::SetMesh(SkeletalMesh* mesh)
{
	meshInstance_->SetMesh(mesh);
}
