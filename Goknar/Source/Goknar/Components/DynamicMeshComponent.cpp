#include "pch.h"

#include "DynamicMeshComponent.h"

#include "Goknar/Model/DynamicMesh.h"
#include "Goknar/Model/DynamicMeshInstance.h"
#include "Goknar/Log.h"
#include "Goknar/Model/MeshInstance.h"

DynamicMeshComponent::DynamicMeshComponent(Component* parent) : MeshComponent(parent)
{}

DynamicMeshComponent::~DynamicMeshComponent()
{

}

void DynamicMeshComponent::SetMesh(DynamicMesh* mesh)
{
	meshInstance_->SetMesh(mesh);
}
