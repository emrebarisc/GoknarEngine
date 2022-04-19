#include "pch.h"

#include "Goknar/Model/DynamicMesh.h"
#include "DynamicMeshComponent.h"

#include "Goknar/Model/DynamicMeshInstance.h"
#include "Goknar/Log.h"
#include "Goknar/Model/MeshInstance.h"

DynamicMeshComponent::DynamicMeshComponent(ObjectBase* parent) : 
	MeshComponent(parent, new DynamicMeshInstance(this))
{}

DynamicMeshComponent::~DynamicMeshComponent()
{

}

void DynamicMeshComponent::SetMesh(Mesh* mesh)
{
	dynamicMesh_ = dynamic_cast<DynamicMesh*>(mesh);
	GOKNAR_ASSERT(dynamicMesh_, "DynamicMeshComponent::SetMesh(Mesh*) assigned mesh is not a DynamicMesh!");
	meshInstance_->SetMesh(mesh);
}
