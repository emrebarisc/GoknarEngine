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

void ProjectileMovementComponent::BeginGame()
{
	Component::BeginGame();
}

void ProjectileMovementComponent::TickComponent(float deltaTime)
{
	Component::TickComponent(deltaTime);

	GOKNAR_INFO("Arrow forward vector: {}", GetOwner()->GetForwardVector().ToString());

	//ObjectBase* owner = GetOwner();

	//Vector3 movementVector = GetOwner()->GetRootComponent()->GetForwardVector();// -Vector3(0.f, 0.f, 0.098f * deltaTime * deltaTime);
	//movementVector *= movementSpeed_;

	//owner->SetWorldPosition(owner->GetWorldPosition() + movementVector * deltaTime);
	//owner->SetWorldRotation(movementVector.GetNormalized().GetRotationNormalized());
}