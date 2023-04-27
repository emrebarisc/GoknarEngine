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
	SetIsActive(true);
}

void ProjectileMovementComponent::BeginGame()
{
	Component::BeginGame();
}

void ProjectileMovementComponent::TickComponent(float deltaTime)
{
	Component::TickComponent(deltaTime);

	//ObjectBase* owner = GetOwner();

	//Vector3 movementVector = owner->GetForwardVector();// -Vector3(0.f, 0.f, 0.098f * deltaTime * deltaTime);
	//movementVector *= movementSpeed_;

	//owner->SetWorldPosition(owner->GetWorldPosition() + movementVector * deltaTime);
	//owner->SetWorldRotation(movementVector.GetNormalized().GetRotationNormalized());
}