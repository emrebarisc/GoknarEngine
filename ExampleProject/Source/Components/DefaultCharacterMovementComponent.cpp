#include "DefaultCharacterMovementComponent.h"

#include "Goknar/Camera.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Components/CameraComponent.h"

#include "Characters/DefaultCharacter.h"

DefaultCharacterMovementComponent::DefaultCharacterMovementComponent(Component* parent) :
	PhysicsMovementComponent(parent)
{
	SetMovementSpeed(WALK_SPEED);
	SetStepHeight(0.2f);
	SetFallSpeed(100.f);
	SetJumpSpeed(9.f);
}

void DefaultCharacterMovementComponent::PreInit()
{
	PhysicsMovementComponent::PreInit();

	ownerCharacter_ = dynamic_cast<DefaultCharacter*>(GetOwner());
	GOKNAR_CORE_ASSERT(ownerCharacter_, "DefaultCharacterMovementComponent can only be added to an DefaultCharacter object");

	thirdPersonCameraComponent_ = ownerCharacter_->GetThirdPersonCameraComponent();
	thirdPersonCamera_ = thirdPersonCameraComponent_->GetCamera();
	
	movementDirection_.speed = 8.f;
	movementDirection_.OnInterpolation = Delegate<void()>::Create<DefaultCharacterMovementComponent, &DefaultCharacterMovementComponent::OnMovementDirectionInterpolated>(this);
}

void DefaultCharacterMovementComponent::Init()
{
	PhysicsMovementComponent::Init();
}

void DefaultCharacterMovementComponent::PostInit()
{
	PhysicsMovementComponent::PostInit();
}

void DefaultCharacterMovementComponent::BeginGame()
{
	PhysicsMovementComponent::BeginGame();
}

void DefaultCharacterMovementComponent::TickComponent(float deltaTime)
{
	PhysicsMovementComponent::TickComponent(deltaTime);

	if (isStrafing_)
	{
		Vector3 cameraForward = thirdPersonCamera_->GetForwardVector();

		Vector3 lookAtVector = Vector3(cameraForward.x, cameraForward.y, 0.f).GetNormalized();

		if (lookAtVector.Length() > 0.001f)
		{
			ownerCharacter_->SetWorldRotation(lookAtVector.GetRotationNormalized());
		}
	}

	Vector3 currentVelocity = movementDirection_.current;
	ownerCharacter_->UpdateAnimationState(currentVelocity, isStrafing_);
}

void DefaultCharacterMovementComponent::AddMovementDirection(const Vector3& movementDirection)
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

void DefaultCharacterMovementComponent::StartRunning()
{
	SetMovementSpeed(RUN_SPEED);
}

void DefaultCharacterMovementComponent::StopRunning()
{
	SetMovementSpeed(WALK_SPEED);
}

void DefaultCharacterMovementComponent::Jump(const Vector3& v)
{
	PhysicsMovementComponent::Jump(v);
}

void DefaultCharacterMovementComponent::OnMovementDirectionInterpolated()
{
	Vector3 movementVectorThisFrame = movementDirection_.current;

	if (BIGGER_EPSILON < movementDirection_.current.Length())
	{
		Vector3 normalizedMovementVector = movementDirection_.current.GetNormalized();
		Vector3 cameraForward = thirdPersonCamera_->GetForwardVector();
		Vector3 cameraLeft = thirdPersonCamera_->GetLeftVector();

		Vector3 cameraForward2D = Vector3(cameraForward.x, cameraForward.y, 0.f).GetNormalized();
		Vector3 cameraLeft2D = Vector3(cameraLeft.x, cameraLeft.y, 0.f).GetNormalized();

		movementVectorThisFrame = normalizedMovementVector.x * cameraForward2D + normalizedMovementVector.y * cameraLeft2D;

		if (!isStrafing_)
		{
			Vector3 lookAtVector = movementVectorThisFrame.GetNormalized();
			lookAtVector.RotateVector(Vector3::UpVector * movementRotation_.current);
			ownerCharacter_->SetWorldRotation(lookAtVector.GetRotationNormalized());
		}
	}

	SetMovementDirection(movementVectorThisFrame);
}

void DefaultCharacterMovementComponent::OnMovementRotationInterpolated()
{

}