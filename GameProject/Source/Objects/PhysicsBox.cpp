#include "PhysicsBox.h"

#include "Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Managers/ResourceManager.h"
#include "Physics/PhysicsDebugger.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/Components/BoxCollisionComponent.h"

#include "Debug/DebugDrawer.h"


#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "Game.h"

PhysicsBox::PhysicsBox() : RigidBody()
{
	boxCollisionComponent_ = AddSubComponent<BoxCollisionComponent>();
	boxCollisionComponent_->SetHalfSize(Vector3{1.f, 1.f, 1.f});

	staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();

	StaticMesh* cubeStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Cube.fbx");
	staticMeshComponent_->SetMesh(cubeStaticMesh);
	staticMeshComponent_->SetParent(boxCollisionComponent_);

	SetMass(100.f);
	// SetLinearFactor(Vector3{0.f, 1.f, 1.f});
	// SetAngularFactor(Vector3{1.f, 1.f, 0.f});

	SetCollisionGroup(CollisionGroup::WorldDynamicBlock);
	SetCollisionMask(CollisionMask::BlockAll);

	SetWorldScaling(Vector3{ 0.5f });

	SetName("PhysicsBox");
}

void PhysicsBox::BeginGame()
{
	RigidBody::BeginGame();


	if (dynamic_cast<Game*>(engine->GetApplication())->GetDrawDebugObjects())
	{
		DebugDrawer::DrawCollisionComponent(boxCollisionComponent_, Colorf::Yellow, 5.f, 0.5f);
	}
}
