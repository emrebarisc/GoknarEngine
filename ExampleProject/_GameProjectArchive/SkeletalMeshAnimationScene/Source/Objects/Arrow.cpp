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
	staticMeshComponent_->SetMesh(arrowStaticMesh);

	movementComponent_ = AddSubComponent<ProjectileMovementComponent>();
	movementComponent_->SetIsActive(false);

	//AxisObject* axisObject = new AxisObject();
	//axisObject->SetParent(this);
}

void Arrow::BeginGame()
{
	ObjectBase::BeginGame();
}

void Arrow::Shoot()
{
	movementComponent_->Shoot();
}
