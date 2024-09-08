#include "ArcherPhysicsMovementComponent.h"

#include "Goknar/Components/CameraComponent.h"

#include "ArcherCharacter.h"

ArcherPhysicsMovementComponent::ArcherPhysicsMovementComponent(Component* parent) :
	PhysicsMovementComponent(parent)
{
}

void ArcherPhysicsMovementComponent::PreInit()
{
	PhysicsMovementComponent::PreInit();

	ownerArcher_ = dynamic_cast<ArcherCharacter*>(GetOwner());
	GOKNAR_CORE_ASSERT(ownerArcher_, "ArcherPhysicsMovementComponent can only be added to an ArcherCharacter object");

	thirdPersonCameraComponent_ = ownerArcher_->GetThirdPersonCameraComponent();
	
	movementDirection_.speed = 8.f;
	movementDirection_.OnInterpolation = Delegate<void()>::create<ArcherPhysicsMovementComponent, &ArcherPhysicsMovementComponent::OnMovementDirectionInterpolated>(this);
}

void ArcherPhysicsMovementComponent::Init()
{
	PhysicsMovementComponent::Init();
}

void ArcherPhysicsMovementComponent::PostInit()
{
	PhysicsMovementComponent::PostInit();
}

void ArcherPhysicsMovementComponent::BeginGame()
{
	PhysicsMovementComponent::BeginGame();
}

void ArcherPhysicsMovementComponent::TickComponent(float deltaTime)
{
	PhysicsMovementComponent::TickComponent(deltaTime);
}

void ArcherPhysicsMovementComponent::AddMovementDirection(const Vector3& movementDirection)
{
	Vector3 newMovementDirection = movementDirection_.destination + movementDirection;

	if (EPSILON < newMovementDirection.Length())
	{
		const float newDestinationAngle = atan2f(newMovementDirection.y, newMovementDirection.x);

		float destinationAngle = newDestinationAngle - movementRotation_.current;
		if (PI < destinationAngle)
		{
			destinationAngle -= TWO_PI;
		}
		else if (destinationAngle <= -PI)
		{
			destinationAngle += TWO_PI;
		}

		movementRotation_ = destinationAngle;
	}

	movementDirection_ = newMovementDirection;
}

void ArcherPhysicsMovementComponent::OnMovementDirectionInterpolated()
{
	Vector3 movementVectorThisFrame = movementDirection_.current;

	if (BIGGER_EPSILON < movementDirection_.current.Length())
	{
		Vector3 normalizedMovementVector = movementDirection_.current.GetNormalized();

		Vector3 cameraForwardVector = thirdPersonCameraComponent_->GetRelativeForwardVector();
		Vector3 cameraForwardVector2D = Vector3(cameraForwardVector.x, cameraForwardVector.y, 0.f).GetNormalized();

		Vector3 cameraLeftVector = thirdPersonCameraComponent_->GetRelativeLeftVector();
		Vector3 cameraLeftVector2D = Vector3(cameraLeftVector.x, cameraLeftVector.y, 0.f).GetNormalized();

		movementVectorThisFrame = normalizedMovementVector.x * cameraForwardVector2D + normalizedMovementVector.y * cameraLeftVector2D;

		Vector3 lookAtVector = movementVectorThisFrame.GetNormalized();
		lookAtVector.RotateVector(Vector3::UpVector * movementRotation_.current);

		ownerArcher_->SetWorldRotation(lookAtVector.GetRotationNormalized());

		ownerArcher_->RunForward();
	}
	else
	{
		ownerArcher_->Idle();
	}

	SetMovementDirection(movementVectorThisFrame);
}

void ArcherPhysicsMovementComponent::OnMovementRotationInterpolated()
{

}