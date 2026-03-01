#include "Characters/DefaultCharacter.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"

#include "Goknar/Components/CameraComponent.h"
#include "Goknar/Components/SkeletalMeshComponent.h"
#include "Goknar/Components/SocketComponent.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Physics/PhysicsWorld.h"
#include "Goknar/Physics/Components/CapsuleCollisionComponent.h"

#include "Controllers/DefaultCharacterController.h"
#include "Components/DefaultCharacterMovementComponent.h"
#include "Objects/Bullet.h"
#include "Objects/Weapon.h"

DefaultCharacter::DefaultCharacter() :
	BaseCharacter()
{
	SetIsTickable(true);

	SetName("DefaultCharacter");
	SetTag("Character");

	capsuleCollisionComponent_->SetRadius(0.4f);
	capsuleCollisionComponent_->SetHeight(0.9f);
	capsuleCollisionComponent_->SetCollisionGroup(GetCollisionGroup());

	defaultCharacterMovementComponent_ = AddSubComponent<DefaultCharacterMovementComponent>();
	defaultCharacterMovementComponent_->SetParent(GetRootComponent());
	movementComponent_ = defaultCharacterMovementComponent_;

	controller_ = new DefaultCharacterController(this);

	skeletalMesh_ = engine->GetResourceManager()->GetContent<SkeletalMesh>("Meshes/SK_DefaultCharacter.fbx");
	skeletalMesh_->GetMaterial()->SetSpecularReflectance( Vector3{1.f} );
	skeletalMesh_->GetMaterial()->SetBaseColor( Vector3{1.f} );
	skeletalMesh_->GetMaterial()->SetAmbientReflectance( Vector3{1.f} );

	skeletalMeshComponent_->SetMesh(skeletalMesh_);
	skeletalMeshComponent_->SetParent(GetRootComponent());
	skeletalMeshComponent_->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3{ 90.f, 0.f, 90.f }));
	skeletalMeshComponent_->SetRelativeScaling(Vector3{ 0.01f });
	skeletalMeshComponent_->SetRelativePosition(Vector3{ 0.f, 0.f, -0.775f });

	thirdPersonCameraComponent_ = AddSubComponent<CameraComponent>();
	thirdPersonCameraComponent_->SetCameraFollowsComponentRotation(false);
	thirdPersonCameraComponent_->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3{ 0.f, 0.f, 0.f }));
	thirdPersonCameraComponent_->SetRelativePosition(Vector3::UpVector * 2.f - Vector3::LeftVector + thirdPersonCameraComponent_->GetRelativeForwardVector() * -8.f);
	thirdPersonCameraComponent_->SetParent(GetRootComponent());

	Camera* thirdPersonCamera = thirdPersonCameraComponent_->GetCamera();
	thirdPersonCamera->SetNearDistance(1.f);
	thirdPersonCamera->SetFarDistance(1000.f);
	thirdPersonCamera->SetFOV(45.f);

	SocketComponent* socketComponent = skeletalMeshComponent_->GetMeshInstance()->AddSocketToBone("mixamorig:RightHand");
	socketComponent->SetRelativePosition(Vector3{ 0.f, 12.5f, 5.f });
	socketComponent->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3{ 90.f, 0.f, 110.f }));
	socketComponent->SetRelativeScaling(Vector3{ 100.f });

	weapon_ = new Weapon();
	weapon_->AttachToSocket(socketComponent);
}

DefaultCharacter::~DefaultCharacter()
{
}

void DefaultCharacter::BeginGame()
{
	RaycastData raycastData;
	raycastData.from = Vector3{0.f, 0.f, 1000.f};
	raycastData.to = Vector3{0.f, 0.f, -1000.f};

	RaycastSingleResult raycastResult;

	engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult);

	SetWorldPosition(raycastResult.hitPosition);

	Idle();

	cameraDistance_.Reset(defaultCameraDistance_);
	cameraDistance_.speed = 32.f;

	cameraHeightOffset_.Reset(Vector3{ 0.f, 0.f, 1.f });
	cameraHeightOffset_.speed = 8.f;
}

void DefaultCharacter::Tick(float deltaTime)
{
	BaseCharacter::Tick(deltaTime);

	const Vector2& cursorMovement = ((DefaultCharacterController*)controller_)->GetCursorDeltaMoveLastFrame();
	float multiplier = mouseSensitivity_ * deltaTime;

	Vector3 forwardVector = thirdPersonCameraComponent_->GetRelativeForwardVector();
	Vector3 leftVector = thirdPersonCameraComponent_->GetRelativeLeftVector();

	Vector3 newForwardVector = forwardVector.RotateVectorAroundAxis(Vector3::UpVector, cursorMovement.x);

	if ((forwardVector.z < 0.25f && 0.f < cursorMovement.y) ||
		(-0.9f < forwardVector.z && cursorMovement.y < 0.f))
	{
		newForwardVector = newForwardVector.RotateVectorAroundAxis(leftVector, -cursorMovement.y);
	}

	thirdPersonCameraComponent_->SetRelativeRotation(newForwardVector.GetRotationNormalized());

	float shoulderOffsetAmount = 0.5f;
	Vector3 rightOffsetVector = -leftVector * shoulderOffsetAmount;

	float desiredMaxDistance = isStrafing_ ? strafingCameraDistance_ : defaultCameraDistance_;

	Vector3 pivotWorldPosition = GetWorldPosition() + cameraHeightOffset_.current + rightOffsetVector;

	Vector3 raycastStart = pivotWorldPosition;
	Vector3 raycastEnd = pivotWorldPosition - (newForwardVector * desiredMaxDistance);

	RaycastData raycastData;
	raycastData.from = raycastStart;
	raycastData.to = raycastEnd;
	raycastData.collisionGroup = CollisionGroup::AllBlock;
	raycastData.collisionMask = CollisionMask::BlockAllExceptCharacter;

	RaycastSingleResult raycastResult;
	bool isHit = engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult);

	if (isHit)
	{
		float hitDistance = (raycastResult.hitPosition - pivotWorldPosition).Length();

		float targetDist = GoknarMath::Max(hitDistance - 0.2f, 0.5f);
		cameraDistance_.UpdateDestination(targetDist);
	}
	else
	{
		cameraDistance_.UpdateDestination(desiredMaxDistance);
	}

	Vector3 newRelativePosition = (cameraHeightOffset_.current + rightOffsetVector) - (newForwardVector * cameraDistance_.current);
	thirdPersonCameraComponent_->SetRelativePosition(newRelativePosition);
}

void DefaultCharacter::Die()
{
	BaseCharacter::Die();
}

void DefaultCharacter::Fire()
{
	if (!isStrafing_)
	{
		return;
	}

	Camera* camera = thirdPersonCameraComponent_->GetCamera();

	Vector3 cameraForward = camera->GetForwardVector();
	Vector3 cameraPosition = camera->GetPosition();
	const float rayLength = 100.f;

	RaycastData raycastData;
	raycastData.from = cameraPosition;
	raycastData.to = cameraPosition + rayLength * cameraForward;

	RaycastSingleResult raycastResult;
	
	PhysicsWorld* physicsWorld = engine->GetPhysicsWorld();
	
	if (physicsWorld->RaycastClosest(raycastData, raycastResult))
	{
		RigidBody* hitRigidBody = dynamic_cast<RigidBody*>(raycastResult.hitObject);
		if (hitRigidBody)
		{
			const Vector3 relativePosition = hitRigidBody->GetWorldPositionInRelativeSpace(raycastResult.hitPosition);
			const float forceMagnitude = 50.f;
			hitRigidBody->ApplyImpulse(forceMagnitude * cameraForward, relativePosition);
		}
	}
}

void DefaultCharacter::ToggleCrouch()
{
    isCrouched_ = !isCrouched_;
}

void DefaultCharacter::SetIsStrafing(bool isStrafing)
{
	isStrafing_ = isStrafing;

	if (movementComponent_)
	{
		defaultCharacterMovementComponent_->SetIsStrafing(isStrafing);
	}
}

void DefaultCharacter::UpdateAnimationState(const Vector3& worldVelocity, bool isStrafing)
{
	float velocityMagnitude = worldVelocity.Length();

	if (GoknarMath::Abs(velocityMagnitude) < EPSILON)
	{
		Idle();

		return;
	}

	Camera* camera = thirdPersonCameraComponent_->GetCamera();
	
	Vector3 cameraForward = camera->GetForwardVector();
	Vector3 cameraLeft = camera->GetLeftVector();

	Vector3 relativeInput = (cameraForward * worldVelocity.x) + (cameraLeft * worldVelocity.y);
    Vector3 direction = relativeInput.GetNormalized();

    bool isRunning = (DefaultCharacterMovementComponent::WALK_SPEED * 1.5f) < velocityMagnitude;

    std::string animationName = "";
	if (isStrafing)
	{
		if (isCrouched_)
		{
			animationName = GetStrafingCrouchAnimation(direction);
		}
		else
		{
			animationName = GetStrafingStandAnimation(direction, isRunning);
		}
	}
	else
	{
		if (isCrouched_)
		{
			animationName = GetCrouchAnimation(direction);
		}
		else
		{
			animationName = GetStandAnimation(direction, isRunning);
		}
	}

    if (!animationName.empty())
    {
        skeletalMeshComponent_->GetMeshInstance()->PlayAnimation(animationName);
    }
}

std::string DefaultCharacter::GetStandAnimation(const Vector3& dir, bool isRunning)
{
	if (isRunning)
	{
		return "Armature|RifleSprintForward";
	}
	else
	{
		return "Armature|RifleWalkForward";
	}
}

std::string DefaultCharacter::GetCrouchAnimation(const Vector3& dir)
{
	return "Armature|RifleCrouchWalkForward";
}

std::string DefaultCharacter::GetStrafingStandAnimation(const Vector3& direction, bool isRunning)
{
    float forward = direction.x;
    float left = direction.y;

    bool isForward = 0.5f < forward;
    bool isBackward = forward < -0.5f;
	bool isLeft = 0.5f < left;
	bool isRight = left < -0.5f;

    if (isRunning)
    {
        if (isForward && isLeft){ return "Armature|RifleRunLeftForward"; }
        if (isForward && isRight){ return "Armature|RifleRunRightForward"; }
        if (isBackward && isLeft){ return "Armature|RifleRunLeftBackward"; }
        if (isBackward && isRight){ return "Armature|RifleRunRightBackward"; }

        if (isForward){ return "Armature|RifleRunForward"; }
        if (isBackward){ return "Armature|RifleRunBackward"; }
        if (isLeft){ return "Armature|RifleRunLeft"; }
        if (isRight){ return "Armature|RifleRunRight"; }
    }

    if (isForward && isLeft){ return "Armature|RifleWalkLeftForward"; }
    if (isForward && isRight){ return "Armature|RifleWalkRighttForward"; }
    if (isBackward && isLeft){ return "Armature|RifleWalkLeftBackward"; }
    if (isBackward && isRight){ return "Armature|RifleWalkRightBackward"; }

    if (isForward){ return "Armature|RifleWalkForward"; }
    if (isBackward){ return "Armature|RifleWalkBackward"; }
    if (isLeft){ return "Armature|RifleWalkLeft"; }
    if (isRight){ return "Armature|RifleWalkRight"; }

    return "Armature|RifleWalkForward";
}

std::string DefaultCharacter::GetStrafingCrouchAnimation(const Vector3& direction)
{
	if (!isStrafing_)
	{
		return "Armature|RifleWalkForward";
	}

    float forward = direction.x;
    float left = direction.y;

    bool isForward = forward > 0.5f;
    bool isBackward = forward < -0.5f;
	bool isLeft = 0.5f < left;
    bool isRight = left < -0.5f;

    if (isForward && isRight){ return "Armature|RifleCrouchWalkRightForward"; }
    if (isForward && isLeft){ return "Armature|RifleCrouchWalkLeftForward"; }
    if (isBackward && isRight){ return "Armature|RifleCrouchWalkRightBackward"; }
    if (isBackward && isLeft){ return "Armature|RifleCrouchWalkLeftBackward"; }

    if (isForward){ return "Armature|RifleCrouchWalkForward"; }
    if (isBackward){ return "Armature|RifleCrouchWalkBackward"; }
    if (isRight){ return "Armature|RifleCrouchWalkRight"; }
    if (isLeft){ return "Armature|RifleCrouchWalkLeft"; }

    return "Armature|RifleCrouchWalkForward";
}

void DefaultCharacter::Idle()
{
    if (isCrouched_)
    {
        skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("Armature|RifleCourchIdle");
    }
    else
    {
        skeletalMeshComponent_->GetMeshInstance()->PlayAnimation("Armature|RifleIdle");
    }
}