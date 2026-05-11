#include "pch.h"

#include "DynamicMeshComponent.h"

#include "Goknar/Model/DynamicMesh.h"
#include "Goknar/Model/DynamicMeshInstance.h"
#include "Goknar/Log.h"
#include "Goknar/Model/IMeshInstance.h"

DynamicMeshComponent::DynamicMeshComponent(Component* parent) : MeshComponent(parent)
{}

DynamicMeshComponent::~DynamicMeshComponent()
{

}

Component* DynamicMeshComponent::Clone() const
{
	DynamicMeshComponent* clonedComponent = new DynamicMeshComponent((Component*)nullptr);
	CopyMeshComponentValuesTo(clonedComponent);

	return clonedComponent;
}

void DynamicMeshComponent::SetMesh(DynamicMesh* mesh)
{
	meshInstance_->SetMesh(mesh);
}
