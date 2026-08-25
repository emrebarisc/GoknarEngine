#include "pch.h"

#include "SkeletalMeshComponent.h"

#include "Goknar/Model/MeshContainer.h"
#include "Goknar/Model/SkeletalMeshInstance.h"
#include "Goknar/Log.h"

SkeletalMeshComponent::SkeletalMeshComponent(Component* parent) : MeshComponent(parent)
{
}

SkeletalMeshComponent::~SkeletalMeshComponent()
{

}

Component* SkeletalMeshComponent::Clone() const
{
	SkeletalMeshComponent* clonedComponent = new SkeletalMeshComponent((Component*)nullptr);
	CopyMeshComponentValuesTo(clonedComponent);

	return clonedComponent;
}

void SkeletalMeshComponent::SetMesh(SkeletalMeshContainer* mesh)
{
	meshInstance_->SetMesh(mesh);
}
