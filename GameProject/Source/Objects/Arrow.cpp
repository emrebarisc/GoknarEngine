#include "Arrow.h"

#include "Goknar/Engine.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Components/StaticMeshComponent.h"

#include "Components/ProjectileMovementComponent.h"
#include "Objects/AxisObject.h"

#include "Physics/Components/SphereCollisionComponent.h"

Arrow::Arrow() : StaticMeshObject()
{
	StaticMesh* arrowStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Arrow.fbx");
	staticMeshComponent_->SetMesh(arrowStaticMesh);

	movementComponent_ = AddSubComponent<ProjectileMovementComponent>();
	movementComponent_->SetIsActive(false);

	// overlappingCollisionComponent_ = AddSubComponent<SphereCollisionComponent>();
	// overlappingCollisionComponent_->SetRadius(2.f);
	// overlappingCollisionComponent_->SetIsOverlapping(true);
	// overlappingCollisionComponent_->OnOverlapBegin = Delegate<OverlapCollisionAlias>::create<Arrow, &Arrow::OnOverlapBegin>(this);
	// overlappingCollisionComponent_->OnOverlapContinue = Delegate<OverlapCollisionAlias>::create<Arrow, &Arrow::OnOverlapContinue>(this);
	// overlappingCollisionComponent_->OnOverlapEnd = Delegate<OverlapCollisionAlias>::create<Arrow, &Arrow::OnOverlapEnd>(this);

	AxisObject* axisObject = new AxisObject();
	axisObject->SetParent(this);
}

void Arrow::BeginGame()
{
	ObjectBase::BeginGame();
}

void Arrow::Shoot()
{
	movementComponent_->Shoot();
}


void Arrow::OnOverlapBegin(ObjectBase* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal)
{
	GOKNAR_INFO("Arrow started overlapped with {} ", otherObject->GetName());
}

void Arrow::OnOverlapContinue(ObjectBase* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal)
{
	GOKNAR_INFO("Arrow continue overlapping with {} ", otherObject->GetName());
}

void Arrow::OnOverlapEnd(ObjectBase* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal)
{
	GOKNAR_INFO("Arrow ended overlapped with {} ", otherObject->GetName());
}