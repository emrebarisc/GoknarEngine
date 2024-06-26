#include "ProjectileMovementComponent.h"

#include "Goknar/Camera.h"
#include "Goknar/Components/SkeletalMeshComponent.h"
#include "Goknar/Model/SkeletalMeshInstance.h"
#include "Goknar/Log.h"

#include "Archer.h"

ProjectileMovementComponent::ProjectileMovementComponent(Component* parent) :
	Component(parent)
{
	SetIsTickable(true);
}

void ProjectileMovementComponent::Shoot()
{
	velocity_ = GetOwner()->GetForwardVector();
	SetIsActive(true);
}

void ProjectileMovementComponent::BeginGame()
{
	Component::BeginGame();
}

void ProjectileMovementComponent::TickComponent(float deltaTime)
{
	Component::TickComponent(deltaTime);

	ObjectBase* owner = GetOwner();

	velocity_ -= Vector3(0.f, 0.f, 9.8f * deltaTime * deltaTime);
	owner->SetWorldPosition(owner->GetWorldPosition() + (velocity_ * movementSpeed_) * deltaTime);
	//owner->SetWorldRotation(movementVector.GetNormalized().GetRotationNormalized());
}