#include "ArcherMovementComponent.h"

#include "Goknar/Camera.h"
#include "Goknar/Components/SkeletalMeshComponent.h"
#include "Goknar/Model/SkeletalMeshInstance.h"
#include "Goknar/Log.h"

#include "Archer.h"

ArcherMovementComponent::ArcherMovementComponent(ObjectBase* parent) :
	Component(parent)
{
	SetIsTickable(true);

	ownerArcher_ = dynamic_cast<Archer*>(parent);
	GOKNAR_ASSERT(ownerArcher_);

	archerSkeletalMeshComponent_ = ownerArcher_->GetSkeletalMeshComponent();
	thirdPersonCamera_ = ownerArcher_->GetThirdPersonCamera();

	movementVector_ = Vector3::ZeroVector;
	movementSpeed_ = 10.f;
}

void ArcherMovementComponent::BeginGame()
{
	Component::BeginGame();

}

void ArcherMovementComponent::TickComponent(float deltaTime)
{
	Component::TickComponent(deltaTime);
	
	if (EPSILON < movementVector_.Length())
	{
		Vector3 normalizedMovementVector = movementVector_.GetNormalized();

		Vector3 cameraForwardVector = thirdPersonCamera_->GetForwardVector();
		Vector3 cameraForwardVector2D = Vector3(cameraForwardVector.x, cameraForwardVector.y, 0.f).GetNormalized();

		Vector3 cameraLeftVector = thirdPersonCamera_->GetLeftVector();
		Vector3 cameraLeftVector2D = Vector3(cameraLeftVector.x, cameraLeftVector.y, 0.f).GetNormalized();

		ownerArcher_->SetWorldPosition(
			ownerArcher_->GetWorldPosition() + 
			(normalizedMovementVector.x * cameraForwardVector2D - normalizedMovementVector.y * cameraLeftVector2D) * movementSpeed_ * deltaTime);
		ownerArcher_->SetWorldRotation(cameraForwardVector2D.GetRotation());

		dynamic_cast<SkeletalMeshInstance*>(ownerArcher_->GetSkeletalMeshComponent()->GetMeshInstance())->PlayAnimation("Armature|StandingRunForward");
	}
	else
	{
		dynamic_cast<SkeletalMeshInstance*>(ownerArcher_->GetSkeletalMeshComponent()->GetMeshInstance())->PlayAnimation("Armature|Idle");
	}
}
