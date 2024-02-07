#include "CannonBall.h"

#include "Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Managers/ResourceManager.h"
#include "Physics/Components/SphereCollisionComponent.h"

CannonBall::CannonBall() : RigidBody()
{
	sphereCollisionComponent_ = AddSubComponent<SphereCollisionComponent>();
	sphereCollisionComponent_->SetRadius(0.5f);

	staticMeshComponent_ = AddSubComponent<StaticMeshComponent>();

	StaticMesh* halfRadiusSphereStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_HalfRadiusSphere.fbx");
	staticMeshComponent_->SetMesh(halfRadiusSphereStaticMesh);
	staticMeshComponent_->SetParent(sphereCollisionComponent_);

    MaterialInstance* materialInstance = MaterialInstance::Create(halfRadiusSphereStaticMesh->GetMaterial());
	materialInstance->SetDiffuseReflectance(Vector3{0.f, 1.f, 0.f});
	staticMeshComponent_->GetMeshInstance()->SetMaterial(materialInstance);

	// overlappingCollisionComponent_ = AddSubComponent<SphereCollisionComponent>();
	// overlappingCollisionComponent_->SetRadius(2.f);
	// overlappingCollisionComponent_->SetIsOverlapping(true);
	// overlappingCollisionComponent_->OnOverlapBegin = Delegate<OverlapCollisionAlias>::create<CannonBall, &CannonBall::OnOverlapBegin>(this);
	// overlappingCollisionComponent_->OnOverlapContinue = Delegate<OverlapCollisionAlias>::create<CannonBall, &CannonBall::OnOverlapContinue>(this);
	// overlappingCollisionComponent_->OnOverlapEnd = Delegate<OverlapCollisionAlias>::create<CannonBall, &CannonBall::OnOverlapEnd>(this);
	// overlappingCollisionComponent_->SetCollisionGroup(CollisionGroup::WorldDynamic);
	// overlappingCollisionComponent_->SetCollisionMask(CollisionMask::BlockWorldDynamic);

	SetMass(100.f);

	SetCollisionGroup(CollisionGroup::WorldDynamic);
	SetCollisionMask(CollisionMask::BlockAll);
	SetCcdMotionThreshold(0.1f);
  	SetCcdSweptSphereRadius(0.05f);

	SetName("CannonBall");
}

void CannonBall::BeginGame()
{
	ObjectBase::BeginGame();
}

void CannonBall::OnOverlapBegin(ObjectBase* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal)
{
	GOKNAR_INFO("Cannon ball started overlapped with {} ", otherObject->GetName());
}

void CannonBall::OnOverlapContinue(ObjectBase* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal)
{
	GOKNAR_INFO("Cannon ball continue overlapping with {} ", otherObject->GetName());
}

void CannonBall::OnOverlapEnd(ObjectBase* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal)
{
	GOKNAR_INFO("Cannon ball ended overlapped with {} ", otherObject->GetName());
}