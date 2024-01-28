#include "PhysicsArcherMovementComponent.h"

#include "Goknar/Camera.h"
#include "Goknar/Components/SkeletalMeshComponent.h"
#include "Goknar/Model/SkeletalMeshInstance.h"
#include "Goknar/Log.h"

#include "PhysicsArcher.h"

PhysicsArcherMovementComponent::PhysicsArcherMovementComponent(Component* parent) :
	Component(parent)
{
	SetIsTickable(true);

	movementVector_.Reset(Vector3::ZeroVector);
	movementVector_.speed = 8.f;

	movementRotation_.Reset(0.f);
	movementRotation_.speed = 4.f;

	movementSpeed_ = 10.f;
}

void PhysicsArcherMovementComponent::BeginGame()
{
	Component::BeginGame();

	ownerPhysicsArcher_ = dynamic_cast<PhysicsArcher*>(GetOwner());
	GOKNAR_ASSERT(ownerPhysicsArcher_);

	archerSkeletalMeshComponent_ = ownerPhysicsArcher_->GetSkeletalMeshComponent();
	thirdPersonCamera_ = ownerPhysicsArcher_->GetThirdPersonCamera();
	archerSkeletalMeshInstance_ = ownerPhysicsArcher_->GetSkeletalMeshComponent()->GetMeshInstance();
}

void PhysicsArcherMovementComponent::TickComponent(float deltaTime)
{
	Component::TickComponent(deltaTime);
	
	if (BIGGER_EPSILON < movementVector_.current.Length())
	{
		Vector3 normalizedMovementVector = movementVector_.current.GetNormalized();

		Vector3 cameraForwardVector = thirdPersonCamera_->GetForwardVector();
		Vector3 cameraForwardVector2D = Vector3(cameraForwardVector.x, cameraForwardVector.y, 0.f).GetNormalized();

		Vector3 cameraLeftVector = thirdPersonCamera_->GetLeftVector();
		Vector3 cameraLeftVector2D = Vector3(cameraLeftVector.x, cameraLeftVector.y, 0.f).GetNormalized();

		Vector3 movementVectorThisFrame = normalizedMovementVector.x * cameraForwardVector2D - normalizedMovementVector.y * cameraLeftVector2D;

		Vector3 lookAtVector = movementVectorThisFrame.GetNormalized();
		lookAtVector.RotateVector(Vector3::UpVector * movementRotation_.current);

		movementVectorThisFrame *= deltaTime;
		
		ownerPhysicsArcher_->ApplyCentralImpulse(movementVectorThisFrame * 1000.f);
		ownerPhysicsArcher_->SetWorldRotation(Quaternion::FromEulerDegrees(lookAtVector));

		ownerPhysicsArcher_->RunForward();
	}
	else
	{
		ownerPhysicsArcher_->ClearForces();
		ownerPhysicsArcher_->Idle();
	}

	movementVector_.Interpolate(deltaTime);
	movementRotation_.Interpolate(deltaTime);
}

void PhysicsArcherMovementComponent::SetMovementVector(const Vector3& movementVector)
{
	movementVector_.UpdateDestination(movementVector);

	if (EPSILON < movementVector.Length())
	{
		const float newDestinationAngle = atan2f(movementVector.y, movementVector.x);

		float destinationAngle = newDestinationAngle - movementRotation_.current;
		if (PI < destinationAngle)
		{
			destinationAngle -= TWO_PI;
		}
		else if (destinationAngle <= -PI)
		{
			destinationAngle += TWO_PI;
		}

		movementRotation_.UpdateDestination(destinationAngle);
	}
}
