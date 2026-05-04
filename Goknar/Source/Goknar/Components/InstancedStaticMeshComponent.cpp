#include "pch.h"

#include "InstancedStaticMeshComponent.h"

#include "Goknar/Model/InstancedStaticMesh.h"
#include "Goknar/Model/InstancedStaticMeshInstance.h"

InstancedStaticMeshComponent::InstancedStaticMeshComponent(Component* parent) :
	MeshComponent(parent)
{
}

InstancedStaticMeshComponent::~InstancedStaticMeshComponent()
{
}

void InstancedStaticMeshComponent::SetMesh(InstancedStaticMesh* mesh)
{
	meshInstance_->SetMesh(mesh);
}
