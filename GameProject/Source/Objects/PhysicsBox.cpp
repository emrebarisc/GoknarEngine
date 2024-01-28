#include "PhysicsBox.h"

#include "Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Managers/ResourceManager.h"
#include "Physics/Components/BoxCollisionComponent.h"

PhysicsBox::PhysicsBox() : RigidBody()
{
	boxCollisionComponent_ = AddSubComponent<BoxCollisionComponent>();
	boxCollisionComponent_->SetHalfSize(Vector3{1.f, 1.f, 1.f});

	staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();

	StaticMesh* cubeStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Cube.fbx");
	staticMeshComponent_->SetMesh(cubeStaticMesh);
	staticMeshComponent_->SetParent(boxCollisionComponent_);

	SetMass(100.f);
	SetLinearFactor(Vector3{0.f, 1.f, 1.f});
	SetAngularFactor(Vector3{1.f, 1.f, 0.f});
}

void PhysicsBox::BeginGame()
{
	ObjectBase::BeginGame();
}
