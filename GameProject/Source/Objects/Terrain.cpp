#include "Terrain.h"

#include "Goknar/Engine.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Physics/Components/NonMovingTriangleMeshCollisionComponent.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"

Terrain::Terrain() : RigidBody()
{
	StaticMesh* terrainStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Terrain.fbx");

	collisionComponent_ = AddSubComponent<NonMovingTriangleMeshCollisionComponent>();
	collisionComponent_->SetMesh(terrainStaticMesh);
	SetRootComponent(collisionComponent_);

	staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();
	staticMeshComponent_->SetMesh(terrainStaticMesh);
	staticMeshComponent_->SetParent(collisionComponent_);

	SetWorldPosition(Vector3{0.f, 0.f, -10.f});

	SetCollisionGroup(CollisionGroup::WorldStatic);
	SetCollisionMask(CollisionMask::BlockAll);

	// SetCcdMotionThreshold(1.f);
  	// SetCcdSweptSphereRadius(0.05f);
}