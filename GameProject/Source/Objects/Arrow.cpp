#include "Arrow.h"

#include "Goknar/Engine.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Components/StaticMeshComponent.h"

#include "Components/ProjectileMovementComponent.h"
#include "Objects/AxisObject.h"

#include "Physics/Components/BoxCollisionComponent.h"

Arrow::Arrow() : OverlappingPhysicsObject()
{
	StaticMesh* arrowStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Arrow.fbx");

	overlappingCollisionComponent_ = AddSubComponent<BoxCollisionComponent>();
	overlappingCollisionComponent_->SetHalfSize(arrowStaticMesh->GetAABB().GetSize());
	overlappingCollisionComponent_->OnOverlapBegin = Delegate<OverlapBeginAlias>::create<Arrow, &Arrow::OnOverlapBegin>(this);
	overlappingCollisionComponent_->OnOverlapContinue = Delegate<OverlapContinueAlias>::create<Arrow, &Arrow::OnOverlapContinue>(this);
	overlappingCollisionComponent_->OnOverlapEnd = Delegate<OverlapEndAlias>::create<Arrow, &Arrow::OnOverlapEnd>(this);

	staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();
	staticMeshComponent_->SetMesh(arrowStaticMesh);

	movementComponent_ = AddSubComponent<ProjectileMovementComponent>();
	movementComponent_->SetIsActive(false);

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

void Arrow::OnOverlapBegin(PhysicsObject* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal)
{
	if(otherObject->GetName().find("ObjectBase") != std::string::npos)
	{
		return;
	}

	GOKNAR_INFO("Arrow started overlapping with {} ", otherObject->GetName());

	Quaternion relativeRotation = otherObject->GetWorldRotation() * GetWorldRotation().GetInverse();
	Vector3 relativePosition = GetWorldPosition() - otherObject->GetWorldPosition();
	Vector3 relativeScaling = Vector3{ 1.f } / otherObject->GetWorldScaling();

	movementComponent_->SetIsActive(false);
	SetWorldPosition(hitPosition, false);
	SetWorldRotation(relativeRotation, false);
	SetWorldScaling(relativeScaling);
	SetParent(otherObject);
}

void Arrow::OnOverlapContinue(PhysicsObject* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal)
{
	GOKNAR_INFO("Arrow continue overlapping with {} ", otherObject->GetName());
}

void Arrow::OnOverlapEnd(PhysicsObject* otherObject, CollisionComponent* otherComponent)
{
	GOKNAR_INFO("Arrow ended overlapping with {} ", otherObject->GetName());
}