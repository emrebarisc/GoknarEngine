#include "Arrow.h"

#include "Goknar/Engine.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Components/StaticMeshComponent.h"

#include "Components/ProjectileMovementComponent.h"
#include "Objects/AxisObject.h"

Arrow::Arrow() : StaticMeshObject()
{
	StaticMesh* arrowStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Arrow.fbx");
	staticMeshComponent->SetMesh(arrowStaticMesh);

	movementComponent_ = AddSubComponent<ProjectileMovementComponent>();
	movementComponent_->SetIsActive(false);
}

void Arrow::BeginGame()
{
	ObjectBase::BeginGame();
}

void Arrow::Shoot()
{
	AxisObject* axisObject = new AxisObject();
	axisObject->SetParent(this, false);

	GOKNAR_INFO("Arrow forward vector: {} Angle: {} Euler: {}", GetForwardVector().ToString(), GetWorldRotation().ToString(), GetWorldRotation().ToEuler().ToString());

	movementComponent_->Shoot();
}
