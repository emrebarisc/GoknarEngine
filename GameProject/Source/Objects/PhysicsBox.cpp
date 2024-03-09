#include "PhysicsBox.h"

#include "Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Managers/ResourceManager.h"
#include "Physics/PhysicsDebugger.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/Components/BoxCollisionComponent.h"

#include "Debug/DebugDrawer.h"


#include "BulletCollision/CollisionShapes/btBoxShape.h"

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

	SetName("PhysicsBox");
}

void PhysicsBox::BeginGame()
{
	RigidBody::BeginGame();

	// DebugDrawer::DrawCollisionComponent(boxCollisionComponent_, Colorf::Yellow, 5.f, 1.f, this);
	
	btBoxShape* boxShape = (btBoxShape*)boxCollisionComponent_->GetBulletCollisionShape();

	// engine->GetPhysicsWorld()->GetPhysicsDebugger()->drawBox(
	// 	bulletCollisionObject_->getWorldTransform().getOrigin() - boxShape->getHalfExtentsWithoutMargin(),
	// 	bulletCollisionObject_->getWorldTransform().getOrigin() + boxShape->getHalfExtentsWithoutMargin(),
	// 	PhysicsUtils::FromVector3ToBtVector3(Vector3{ 1.f, 1.f, 1.f })
	// );
}
