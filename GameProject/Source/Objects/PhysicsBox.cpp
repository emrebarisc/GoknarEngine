#include "PhysicsBox.h"

#include "Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Managers/ResourceManager.h"
#include "Physics/Components/BoxCollisionComponent.h"

PhysicsBox::PhysicsBox() : RigidBody()
{
	boxCollisionComponent_ = AddSubComponent<BoxCollisionComponent>();
	boxCollisionComponent_->SetHalfSize(GetWorldScaling() * 0.5f);

	staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();

	StaticMesh* cubeStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Cube.fbx");
	staticMeshComponent_->SetMesh(cubeStaticMesh);
	staticMeshComponent_->SetParent(boxCollisionComponent_);
}

void PhysicsBox::BeginGame()
{
	ObjectBase::BeginGame();
}
