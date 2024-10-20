#include "Terrain.h"

#include "Goknar/Engine.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Physics/Components/NonMovingTriangleMeshCollisionComponent.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"

#include "Debug/DebugDrawer.h"

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
	SetWorldRotation(Quaternion::FromEulerDegrees(Vector3{0.f, 0.f, 90.f}));

	SetCollisionGroup(CollisionGroup::WorldStaticBlock);
	SetCollisionMask(CollisionMask::BlockAll);

	SetMass(0.f);

	SetName("Terrain");
}

void Terrain::BeginGame()
{
	RigidBody::BeginGame();

	//DebugDrawer::DrawCollisionComponent(collisionComponent_, Colorf::Black, 4.f);
}
