#include "pch.h"

#include "SkeletalMeshComponent.h"

#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Model/SkeletalMeshInstance.h"
#include "Goknar/Log.h"

SkeletalMeshComponent::SkeletalMeshComponent(ObjectBase* parent) : 
	MeshComponent(parent, new SkeletalMeshInstance(this)),
	skeletalMesh_(nullptr)
{
}

SkeletalMeshComponent::~SkeletalMeshComponent()
{

}

void SkeletalMeshComponent::SetMesh(Mesh* mesh)
{
	skeletalMesh_ = dynamic_cast<SkeletalMesh*>(mesh);
	GOKNAR_ASSERT(skeletalMesh_, "SkeletalMeshComponent::SetMesh(Mesh*) assigned mesh is not a SkeletalMesh!");
	meshInstance_->SetMesh(skeletalMesh_);
}
