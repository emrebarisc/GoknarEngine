#include "MultipleCollisionComponentObject.h"

#include "Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Managers/ResourceManager.h"
#include "Physics/PhysicsDebugger.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/Components/BoxCollisionComponent.h"
#include "Physics/Components/SphereCollisionComponent.h"
#include "Physics/Components/MultipleCollisionComponent.h"

#include "Debug/DebugDrawer.h"

#include "Game.h"

MultipleCollisionComponentObject::MultipleCollisionComponentObject() : RigidBody()
{
	multipleCollisionComponent_ = AddSubComponent<MultipleCollisionComponent>();

	StaticMesh* halfRadiusSphereStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_HalfRadiusSphere.fbx");

	unitSphereStaticMeshComponent1_ = AddSubComponent<StaticMeshComponent>();
	unitSphereStaticMeshComponent1_->SetMesh(halfRadiusSphereStaticMesh);
	unitSphereStaticMeshComponent1_->SetParent(multipleCollisionComponent_);
	unitSphereStaticMeshComponent1_->SetRelativePosition(Vector3{ 0.f, 0.f, -2.5f });

	unitSphereStaticMeshComponent2_ = AddSubComponent<StaticMeshComponent>();
	unitSphereStaticMeshComponent2_->SetMesh(halfRadiusSphereStaticMesh);
	unitSphereStaticMeshComponent2_->SetParent(multipleCollisionComponent_);
	unitSphereStaticMeshComponent2_->SetRelativePosition(Vector3{ 0.f, 0.f, 2.5f });

	boxStaticMeshComponent_ = AddSubComponent<StaticMeshComponent>();

	StaticMesh* cubeStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Cube.fbx");
	boxStaticMeshComponent_->SetMesh(cubeStaticMesh);
	boxStaticMeshComponent_->SetRelativeScaling(Vector3{ 0.5f, 0.5f, 2.f });

	boxCollisionComponent_ = AddSubComponent<BoxCollisionComponent>();
	boxCollisionComponent_->SetRelativeScaling(Vector3{ 1.f, 1.f, 4.f });
	multipleCollisionComponent_->AddCollisionComponent(boxCollisionComponent_);

	sphereCollisionComponent1_ = AddSubComponent<SphereCollisionComponent>();
	sphereCollisionComponent1_->SetRadius(0.5f);
	sphereCollisionComponent1_->SetRelativePosition(Vector3{ 0.f, 0.f, -2.5f });
	multipleCollisionComponent_->AddCollisionComponent(sphereCollisionComponent1_);

	sphereCollisionComponent2_ = AddSubComponent<SphereCollisionComponent>();
	sphereCollisionComponent2_->SetRadius(0.5f);
	sphereCollisionComponent2_->SetRelativePosition(Vector3{ 0.f, 0.f, 2.5f });
	multipleCollisionComponent_->AddCollisionComponent(sphereCollisionComponent2_);

	SetMass(100.f);

	SetCollisionGroup(CollisionGroup::WorldDynamicBlock);
	SetCollisionMask(CollisionMask::BlockAll);

	SetName("MultipleCollisionComponentObject");
}

void MultipleCollisionComponentObject::BeginGame()
{
	ObjectBase::BeginGame();

	if (dynamic_cast<Game*>(engine->GetApplication())->GetDrawDebugObjects())
	{
		DebugDrawer::DrawCollisionComponent(sphereCollisionComponent1_, Colorf::Green, 5.f, 0.5f);
		DebugDrawer::DrawCollisionComponent(sphereCollisionComponent2_, Colorf::Green, 5.f, 0.5f);
		DebugDrawer::DrawCollisionComponent(boxCollisionComponent_, Colorf::Green, 5.f, 0.5f);
	}
}
