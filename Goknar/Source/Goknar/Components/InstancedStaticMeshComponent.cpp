#include "pch.h"

#include "InstancedStaticMeshComponent.h"

#include "Goknar/Engine.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/InstancedStaticMesh.h"
#include "Goknar/Model/InstancedStaticMeshInstance.h"
#include "Goknar/Model/StaticMesh.h"

InstancedStaticMeshComponent::InstancedStaticMeshComponent(Component* parent) :
	MeshComponent(parent)
{
}

InstancedStaticMeshComponent::~InstancedStaticMeshComponent()
{
}

Component* InstancedStaticMeshComponent::Clone() const
{
	InstancedStaticMeshComponent* clonedComponent = new InstancedStaticMeshComponent((Component*)nullptr);
	CopyValuesTo(clonedComponent);

	InstancedStaticMesh* sourceMesh = meshInstance_ ? meshInstance_->GetMesh() : nullptr;
	if (sourceMesh)
	{
		static unsigned int cloneMeshIndex = 0;
		StaticMesh* sourceStaticMesh = nullptr;
		const std::string& sourceMeshPath = sourceMesh->GetSourceMeshPath();
		if (!sourceMeshPath.empty() && engine && engine->GetResourceManager())
		{
			std::string contentRelativeSourceMeshPath = sourceMeshPath;
			if (contentRelativeSourceMeshPath.find(ContentDir) == 0)
			{
				contentRelativeSourceMeshPath = contentRelativeSourceMeshPath.substr(ContentDir.size());
			}
			sourceStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>(contentRelativeSourceMeshPath);
		}

		if (!sourceStaticMesh)
		{
			sourceStaticMesh = sourceMesh;
		}

		InstancedStaticMesh* clonedMesh = InstancedStaticMesh::CreateFromStaticMesh(
			sourceStaticMesh,
			sourceMesh->GetPath() + "::Clone_" + std::to_string(cloneMeshIndex++));

		if (clonedMesh)
		{
			clonedMesh->SetInstanceTransformations(sourceMesh->GetInstanceTransformationMatrices());
			clonedComponent->SetMesh(clonedMesh);
		}
	}

	if (meshInstance_ && clonedComponent->meshInstance_)
	{
		clonedComponent->meshInstance_->SetRenderMask(meshInstance_->GetRenderMask());
		clonedComponent->meshInstance_->SetIsRendered(meshInstance_->GetIsRendered());
		clonedComponent->meshInstance_->SetIsCastingShadow(meshInstance_->GetIsCastingShadow());
	}

	return clonedComponent;
}

void InstancedStaticMeshComponent::SetMesh(InstancedStaticMesh* mesh)
{
	meshInstance_->SetMesh(mesh);
}
