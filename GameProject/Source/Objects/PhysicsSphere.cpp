#include "PhysicsSphere.h"

#include "Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Managers/ResourceManager.h"
#include "Physics/Components/SphereCollisionComponent.h"

#include "Goknar/Debug/DebugDrawer.h"

PhysicsSphere::PhysicsSphere() : RigidBody()
{
	sphereCollisionComponent_ = AddSubComponent<SphereCollisionComponent>();
	sphereCollisionComponent_->SetRadius(0.5f);

	staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();

	StaticMesh* halfRadiusSphereStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_HalfRadiusSphere.fbx");
	staticMeshComponent_->SetMesh(halfRadiusSphereStaticMesh);
	staticMeshComponent_->SetParent(sphereCollisionComponent_);

	SetMass(100.f);

	SetCollisionGroup(CollisionGroup::WorldDynamicBlock);
	SetCollisionMask(CollisionMask::BlockAll);

	SetName("PhysicsSphere");
}

void PhysicsSphere::BeginGame()
{
	ObjectBase::BeginGame();

	//DebugDrawer::DrawCollisionComponent(sphereCollisionComponent_, Colorf::Red, 5.f, 0.5f);
}
