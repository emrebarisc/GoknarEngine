#include "pch.h"

#include "StaticMeshComponent.h"

#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Model/StaticMeshInstance.h"
#include "Goknar/Log.h"

StaticMeshComponent::StaticMeshComponent(ObjectBase* parent) : 
	MeshComponent(parent, new StaticMeshInstance(this))
{
}

StaticMeshComponent::~StaticMeshComponent()
{

}

void StaticMeshComponent::SetMesh(Mesh* mesh)
{
	staticMesh_ = dynamic_cast<StaticMesh*>(mesh);
	GOKNAR_ASSERT(staticMesh_, "StaticMeshComponent::SetMesh(Mesh*) assigned mesh is not a StaticMesh!");
	meshInstance_->SetMesh(mesh);
}
