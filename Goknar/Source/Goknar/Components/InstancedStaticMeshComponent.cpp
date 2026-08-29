#include "pch.h"

#include "InstancedStaticMeshComponent.h"

#include "Goknar/Engine.h"
#include "Goknar/Helpers/ContentPathUtils.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/InstancedStaticMesh.h"
#include "Goknar/Model/Mesh.h"
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

	InstancedStaticMesh* sourceMeshContainer = meshInstance_ ? meshInstance_->GetMesh() : nullptr;
	InstancedStaticMeshLOD* sourceInstancedMesh = sourceMeshContainer ? sourceMeshContainer->GetLOD(0) : nullptr;
	if (sourceMeshContainer && sourceInstancedMesh)
	{
		std::string ownerMeshPath = sourceMeshContainer->GetPath();
		const size_t instancedMeshPostfixIndex = ownerMeshPath.find("::");
		if (instancedMeshPostfixIndex != std::string::npos)
		{
			ownerMeshPath = ownerMeshPath.substr(0, instancedMeshPostfixIndex);
		}
		ownerMeshPath = ContentPathUtils::ToContentRelativePath(ownerMeshPath);

		StaticMeshLOD* sourceStaticMesh = sourceInstancedMesh;
		if (!ownerMeshPath.empty() && engine && engine->GetResourceManager())
		{
			StaticMesh* ownerMeshContainer = engine->GetResourceManager()->GetContent<StaticMesh>(ownerMeshPath);
			if (ownerMeshContainer && ownerMeshContainer->GetLODCount() > 0 && ownerMeshContainer->GetLOD(0))
			{
				sourceStaticMesh = ownerMeshContainer->GetLOD(0);
			}
		}

		InstancedStaticMeshLOD* clonedMesh = InstancedStaticMeshLOD::CreateFromStaticMesh(
			sourceStaticMesh,
			sourceMeshContainer->GetPath() + "::Clone_" + std::to_string(clonedComponent->GetGUID()));

		if (clonedMesh)
		{
			clonedMesh->SetInstanceTransformations(sourceInstancedMesh->GetInstanceTransformationMatrices());

			InstancedStaticMesh* clonedMeshContainer = new InstancedStaticMesh();
			clonedMeshContainer->SetPath(clonedMesh->GetPath());
			clonedMeshContainer->AddLOD(LODSetting<InstancedStaticMeshLOD>{ clonedMesh, MAX_FLOAT });
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

void InstancedStaticMeshComponent::SetMesh(InstancedStaticMesh* mesh)
{
	meshInstance_->SetMesh(mesh);
}
