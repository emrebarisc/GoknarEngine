#include "pch.h"

#include "StaticMeshComponent.h"

#include "Goknar/Model/StaticMeshInstance.h"

StaticMeshComponent::StaticMeshComponent(ObjectBase* parent) : 
	MeshComponent(parent, new StaticMeshInstance(this))
{
}

StaticMeshComponent::~StaticMeshComponent()
{

}

void StaticMeshComponent::SetMesh(Mesh* mesh)
{
	meshInstance_->SetMesh(mesh);
}
