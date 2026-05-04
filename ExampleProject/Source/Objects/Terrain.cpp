#include "Terrain.h"

#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Physics/Components/NonMovingTriangleMeshCollisionComponent.h"

Terrain::Terrain() : RigidBody()
{
	StaticMesh* terrainStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Terrain.fbx");

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
			staticMeshComponent_->SetMesh(terrainStaticMesh);
		}
	}

	SetCollisionGroup(CollisionGroup::WorldStaticBlock);
	SetCollisionMask(CollisionMask::BlockAll);
	SetMass(0.f);

	SetName("Terrain");
}
