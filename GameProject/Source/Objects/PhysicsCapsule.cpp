#include "PhysicsCapsule.h"

#include "Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Managers/ResourceManager.h"
#include "Physics/PhysicsDebugger.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/Components/CapsuleCollisionComponent.h"

#include "Debug/DebugDrawer.h"

#include "BulletCollision/CollisionShapes/btBoxShape.h"

#include "Game.h"

PhysicsCapsule::PhysicsCapsule() : RigidBody()
{
	capsuleCollisionComponent_ = AddSubComponent<CapsuleCollisionComponent>();
	capsuleCollisionComponent_->SetRadius(0.5f);
	capsuleCollisionComponent_->SetHeight(2.f);

	staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();

	StaticMesh* cubeStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Capsule.fbx");
	staticMeshComponent_->SetMesh(cubeStaticMesh);
	staticMeshComponent_->SetParent(capsuleCollisionComponent_);

	SetMass(100.f);
	// SetLinearFactor(Vector3{0.f, 1.f, 1.f});
	// SetAngularFactor(Vector3{1.f, 1.f, 0.f});

	SetCollisionGroup(CollisionGroup::WorldDynamicBlock);
	SetCollisionMask(CollisionMask::BlockAll);

	SetName("PhysicsCapsule");
}

void PhysicsCapsule::BeginGame()
{
	RigidBody::BeginGame();

	if (dynamic_cast<Game*>(engine->GetApplication())->GetDrawDebugObjects())
	{
		DebugDrawer::DrawCollisionComponent(capsuleCollisionComponent_, Colorf::Blue, 5.f, 0.5f);
	}
}
