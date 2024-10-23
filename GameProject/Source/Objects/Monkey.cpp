#include "Monkey.h"

#include "Goknar/Engine.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Physics/Components/MovingTriangleMeshCollisionComponent.h"

#include "BulletDynamics/Dynamics/btRigidBody.h"

#include "Debug/DebugDrawer.h"

#include "Game.h"

Monkey::Monkey() : RigidBody()
{
	StaticMesh* monkeyCollisionStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Monkey.fbx");// "Meshes/SM_MonkeyCollision.fbx");
	collisionComponent_ = AddSubComponent<MovingTriangleMeshCollisionComponent>();
	collisionComponent_->SetMesh(monkeyCollisionStaticMesh);
	SetRootComponent(collisionComponent_);

	StaticMesh* monkeyStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Monkey.fbx");
	staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();
	staticMeshComponent_->SetMesh(monkeyStaticMesh);
	staticMeshComponent_->SetParent(collisionComponent_);

	SetWorldPosition(Vector3{0.f, 0.f, -10.f});

	SetMass(50.f);

	SetCollisionGroup(CollisionGroup::WorldDynamicBlock);
	SetCollisionMask(CollisionMask::BlockAll);

	SetName("Monkey");
}

void Monkey::BeginGame()
{
	RigidBody::BeginGame();

	if (dynamic_cast<Game*>(engine->GetApplication())->GetDrawDebugObjects())
	{
		DebugDrawer::DrawCollisionComponent(collisionComponent_, Colorf::Magenta, 5.f, 0.5f);
	}
}
