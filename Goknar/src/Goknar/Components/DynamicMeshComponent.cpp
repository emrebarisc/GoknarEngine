#include "pch.h"

#include "DynamicMeshComponent.h"

#include "Goknar/Model/MeshInstance.h"
#include "Goknar/Model/DynamicMeshInstance.h"

DynamicMeshComponent::DynamicMeshComponent(ObjectBase* parent) : 
	MeshComponent(parent, new DynamicMeshInstance(this))
{}

DynamicMeshComponent::~DynamicMeshComponent()
{

}

void DynamicMeshComponent::SetMesh(Mesh* mesh)
{
	meshInstance_->SetMesh(mesh);
}
