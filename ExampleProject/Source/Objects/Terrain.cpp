#include "Terrain.h"

#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/Mesh.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Physics/Components/NonMovingTriangleMeshCollisionComponent.h"

Terrain::Terrain() : RigidBody()
{
	StaticMesh* terrainStaticMeshContainer = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Terrain.fbx");
	StaticMeshLOD* terrainStaticMesh = terrainStaticMeshContainer ? terrainStaticMeshContainer->GetLOD(0) : nullptr;

	if (terrainStaticMesh)
	{
		collisionComponent_ = AddSubComponent<NonMovingTriangleMeshCollisionComponent>();
		if (collisionComponent_)
		{
			collisionComponent_->SetMesh(terrainStaticMesh);
			SetRootComponent(collisionComponent_);
		}

		staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();
		if (staticMeshComponent_)
		{
			staticMeshComponent_->SetMesh(terrainStaticMeshContainer);
		}
	}

	SetCollisionGroup(CollisionGroup::WorldStaticBlock);
	SetCollisionMask(CollisionMask::BlockAll);
	SetMass(0.f);

	SetName("Terrain");
}
