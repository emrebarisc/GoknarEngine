#include "pch.h"

#include "StaticMeshComponent.h"

#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Model/StaticMeshInstance.h"
#include "Goknar/Log.h"

StaticMeshComponent::StaticMeshComponent(Component* parent) : MeshComponent(parent)
{
}

StaticMeshComponent::~StaticMeshComponent()
{

}

void StaticMeshComponent::SetMesh(StaticMesh* mesh)
{
	meshInstance_->SetMesh(mesh);
}
