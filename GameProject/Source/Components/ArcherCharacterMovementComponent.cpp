#include "ArcherCharacterMovementComponent.h"

#include "Goknar/Camera.h"

#include "ArcherCharacter.h"

ArcherCharacterMovementComponent::ArcherCharacterMovementComponent(Component* parent) :
	CharacterMovementComponent(parent)
{
}

void ArcherCharacterMovementComponent::PreInit()
{
	CharacterMovementComponent::PreInit();

	ownerArcher_ = dynamic_cast<ArcherCharacter*>(GetOwner());
	GOKNAR_CORE_ASSERT(ownerArcher_, "ArcherCharacterMovementComponent can only be added to an ArcherCharacter object");

	thirdPersonCamera_ = ownerArcher_->GetThirdPersonCamera();

	movementDirection_.OnInterpolation = Delegate<void()>::create<ArcherCharacterMovementComponent, &ArcherCharacterMovementComponent::OnMovementDirectionInterpolated>(this);
}

void ArcherCharacterMovementComponent::Init()
{
	CharacterMovementComponent::Init();
}

void ArcherCharacterMovementComponent::PostInit()
{
	CharacterMovementComponent::PostInit();
}

void ArcherCharacterMovementComponent::BeginGame()
{
	CharacterMovementComponent::BeginGame();
}

void ArcherCharacterMovementComponent::TickComponent(float deltaTime)
{
	CharacterMovementComponent::TickComponent(deltaTime);
}

void ArcherCharacterMovementComponent::OnMovementDirectionInterpolated()
{
	SetMovementDirection(movementDirection_.current);

	if (BIGGER_EPSILON < movementDirection_.current.Length())
	{
		Vector3 normalizedMovementVector = movementDirection_.current.GetNormalized();

		Vector3 cameraForwardVector = thirdPersonCamera_->GetForwardVector();
		Vector3 cameraForwardVector2D = Vector3(cameraForwardVector.x, cameraForwardVector.y, 0.f).GetNormalized();

		Vector3 cameraLeftVector = thirdPersonCamera_->GetLeftVector();
		Vector3 cameraLeftVector2D = Vector3(cameraLeftVector.x, cameraLeftVector.y, 0.f).GetNormalized();

		Vector3 movementVectorThisFrame = normalizedMovementVector.x * cameraForwardVector2D - normalizedMovementVector.y * cameraLeftVector2D;

		Vector3 lookAtVector = movementVectorThisFrame.GetNormalized();
		lookAtVector.RotateVector(Vector3::UpVector * movementRotation_.current);

		ownerArcher_->SetWorldRotation(lookAtVector.GetRotationNormalized());

		ownerArcher_->RunForward();
	}
	else
	{
		ownerArcher_->Idle();
	}
};