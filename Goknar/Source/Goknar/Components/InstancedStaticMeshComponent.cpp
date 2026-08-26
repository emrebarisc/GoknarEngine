#include "pch.h"

#include "InstancedStaticMeshComponent.h"

#include "Goknar/Engine.h"
#include "Goknar/Helpers/ContentPathUtils.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/InstancedStaticMesh.h"
#include "Goknar/Model/MeshContainer.h"
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

	InstancedStaticMeshContainer* sourceMeshContainer = meshInstance_ ? meshInstance_->GetMesh() : nullptr;
	InstancedStaticMesh* sourceInstancedMesh = sourceMeshContainer ? sourceMeshContainer->GetLOD(0) : nullptr;
	if (sourceMeshContainer && sourceInstancedMesh)
	{
		std::string ownerMeshPath = sourceMeshContainer->GetPath();
		const size_t instancedMeshPostfixIndex = ownerMeshPath.find("::");
		if (instancedMeshPostfixIndex != std::string::npos)
		{
			ownerMeshPath = ownerMeshPath.substr(0, instancedMeshPostfixIndex);
		}
		ownerMeshPath = ContentPathUtils::ToContentRelativePath(ownerMeshPath);

		StaticMesh* sourceStaticMesh = sourceInstancedMesh;
		if (!ownerMeshPath.empty() && engine && engine->GetResourceManager())
		{
			StaticMeshContainer* ownerMeshContainer = engine->GetResourceManager()->GetContent<StaticMeshContainer>(ownerMeshPath);
			if (ownerMeshContainer && ownerMeshContainer->GetLODCount() > 0 && ownerMeshContainer->GetLOD(0))
			{
				sourceStaticMesh = ownerMeshContainer->GetLOD(0);
			}
		}

		InstancedStaticMesh* clonedMesh = InstancedStaticMesh::CreateFromStaticMesh(
			sourceStaticMesh,
			sourceMeshContainer->GetPath() + "::Clone_" + std::to_string(clonedComponent->GetGUID()));

		if (clonedMesh)
		{
			clonedMesh->SetInstanceTransformations(sourceInstancedMesh->GetInstanceTransformationMatrices());

			InstancedStaticMeshContainer* clonedMeshContainer = new InstancedStaticMeshContainer();
			clonedMeshContainer->SetPath(clonedMesh->GetPath());
			clonedMeshContainer->AddLOD(LODSetting<InstancedStaticMesh>{ clonedMesh, MAX_FLOAT });
			if (engine && engine->GetResourceManager() && engine->GetResourceManager()->GetResourceContainer())
			{
				engine->GetResourceManager()->GetResourceContainer()->AddMesh(clonedMeshContainer);
			}
			clonedComponent->SetMesh(clonedMeshContainer);
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

void InstancedStaticMeshComponent::SetMesh(InstancedStaticMeshContainer* mesh)
{
	meshInstance_->SetMesh(mesh);
}
