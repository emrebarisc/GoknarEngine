#include "Characters/DefaultCharacter.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"

#include "Goknar/Animation/AnimationGraph.h"
#include "Goknar/Animation/AnimationNode.h"
#include "Goknar/Animation/AnimationState.h"
#include "Goknar/Animation/AnimationTransition.h"
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

#include "Animation/AnimationSerializer.h"
#include "Animation/AnimationDeserializer.h"

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

	skeletalMesh_ = engine->GetResourceManager()->GetContent<SkeletalMesh>("Meshes/Characters/SK_DefaultCharacter.fbx");

	skeletalMeshComponent_->SetMesh(skeletalMesh_);
	skeletalMeshComponent_->SetParent(GetRootComponent());
	skeletalMeshComponent_->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3{ 90.f, 0.f, 90.f }));
	skeletalMeshComponent_->SetRelativeScaling(Vector3{ 0.01f });
	skeletalMeshComponent_->SetRelativePosition(Vector3{ 0.f, 0.f, -0.775f });

	thirdPersonCameraComponent_ = AddSubComponent<CameraComponent>();
	thirdPersonCameraComponent_->SetCameraFollowsComponentRotation(false);
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

	animationGraph_ = new AnimationGraph();
	animationGraph_->relativeSkeletalMeshInstance = skeletalMeshComponent_->GetMeshInstance();
	AnimationDeserializer().Deserialize(animationGraph_, "Animations/AG_DefaultCharacter");
}

DefaultCharacter::~DefaultCharacter()
{
	delete animationGraph_;
}

void DefaultCharacter::BeginGame()
{
	RaycastData raycastData;
	raycastData.from = Vector3{0.f, 0.f, 1000.f};
	raycastData.to = Vector3{0.f, 0.f, -1000.f};

	RaycastSingleResult raycastResult;

	engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult);

	SetWorldPosition(raycastResult.hitPosition + Vector3{0.f, 0.f, 1.f});

	Quaternion spawnRotation = Quaternion::FromEulerDegrees(Vector3{ 0.f, 0.f, 180.f });
	SetWorldRotation(spawnRotation);
	thirdPersonCameraComponent_->SetRelativeRotation(spawnRotation);

	Idle();

	cameraDistance_.Reset(defaultCameraDistance_);
	cameraDistance_.speed = 32.f;

	cameraHeightOffset_.Reset(Vector3{ 0.f, 0.f, 1.f });
	cameraHeightOffset_.speed = 8.f;

	animationGraph_->Init();
}

void DefaultCharacter::Tick(float deltaTime)
{
	BaseCharacter::Tick(deltaTime);

	const Vector2& cursorMovement = ((DefaultCharacterController*)controller_)->GetCursorDeltaMoveLastFrame();

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

	float animation2DVelocityMagnitude = Vector2(movementComponent_->GetLinearVelocity()).Length();
	animationGraph_->SetVariable<float>("VelocityMagnitude", animation2DVelocityMagnitude);
	animationGraph_->SetVariable<bool>("IsOnGround", movementComponent_->OnGround());

	if (0.f < animation2DVelocityMagnitude)
	{
		Vector3 movementVector = movementComponent_->GetMovementDirection().GetNormalized();
		float dotProduct = (forwardVector.x * movementVector.x) + (forwardVector.y * movementVector.y);
		float determinant = (forwardVector.x * movementVector.y) - (forwardVector.y * movementVector.x);

		animationGraph_->SetVariable<float>("StrafingRotation", GoknarMath::Atan2(determinant, dotProduct));
	}

	animationGraph_->Update(deltaTime);
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

	animationGraph_->SetVariable<bool>("IsCrouched", isCrouched_);

	if (isCrouched_)
	{
		cameraHeightOffset_ = Vector3{0.f, 0.f, 0.5f};
		defaultCharacterMovementComponent_->StartCrouching();
	}
	else
	{
		cameraHeightOffset_ = Vector3{ 0.f, 0.f, 1.f };

		if (((DefaultCharacterController*)controller_)->GetIsRunInputPresent())
		{
			defaultCharacterMovementComponent_->StartRunning();
		}
		else
		{
			defaultCharacterMovementComponent_->StartWalking();
		}
	}
}

void DefaultCharacter::StartRunning()
{
	if (isCrouched_)
	{
		return;
	}

	defaultCharacterMovementComponent_->StartRunning();
}

void DefaultCharacter::StopRunning()
{
	if (isCrouched_)
	{
		return;
	}

	defaultCharacterMovementComponent_->StartWalking();
}

void DefaultCharacter::SetIsStrafing(bool isStrafing)
{
	isStrafing_ = isStrafing;

	animationGraph_->SetVariable<bool>("IsStrafing", isStrafing_);

	if (movementComponent_)
	{
		defaultCharacterMovementComponent_->SetIsStrafing(isStrafing);
	}
}

void DefaultCharacter::Idle()
{
}

void DefaultCharacter::Jump()
{
	if (isCrouched_)
	{
		ToggleCrouch();
		return;
	}

	BaseCharacter::Jump();

	defaultCharacterMovementComponent_->Jump();
	animationGraph_->SetTrigger<bool>("IsJumping", true);
}