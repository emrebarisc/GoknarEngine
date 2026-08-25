#include "pch.h"

#include "StaticMeshComponent.h"

#include "Goknar/Model/MeshContainer.h"
#include "Goknar/Model/StaticMeshInstance.h"
#include "Goknar/Log.h"
#include "Goknar/Helpers/SceneParser.h"

StaticMeshComponent::StaticMeshComponent(Component* parent) : MeshComponent(parent)
{
}

StaticMeshComponent::~StaticMeshComponent()
{

}

Component* StaticMeshComponent::Clone() const
{
	StaticMeshComponent* clonedComponent = new StaticMeshComponent((Component*)nullptr);
	CopyMeshComponentValuesTo(clonedComponent);
	SceneParser::ApplyStaticMeshComponentMaterialPaths(clonedComponent, SceneParser::GetStaticMeshComponentMaterialPaths(this));

	return clonedComponent;
}

void StaticMeshComponent::SetMesh(StaticMeshContainer* mesh)
{
	meshInstance_->SetMesh(mesh);
}
