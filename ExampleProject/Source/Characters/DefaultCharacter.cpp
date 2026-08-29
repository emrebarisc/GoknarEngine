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
#include "Goknar/Model/MeshContainer.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Objects/PlayerStart.h"
#include "Goknar/Physics/PhysicsWorld.h"
#include "Goknar/Physics/Components/CapsuleCollisionComponent.h"

#include "Controllers/DefaultCharacterController.h"
#include "Components/DefaultCharacterMovementComponent.h"
#include "Objects/Bullet.h"
#include "Objects/Weapon.h"
#include "Objects/VFX/BulletHitVFXObject.h"

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
	skeletalMeshComponent_->SetRelativePosition(Vector3{ 0.f, 0.f, -0.8f });

	thirdPersonCameraComponent_ = AddSubComponent<CameraComponent>();
	thirdPersonCameraComponent_->SetCameraFollowsComponentRotation(false);
	thirdPersonCameraComponent_->SetRelativePosition(Vector3::UpVector + (-Vector3::LeftVector * defaultShoulderOffset_) + thirdPersonCameraComponent_->GetRelativeForwardVector() * -defaultCameraDistance_);
	thirdPersonCameraComponent_->SetParent(GetRootComponent());

	Camera* thirdPersonCamera = thirdPersonCameraComponent_->GetCamera();
	thirdPersonCamera->SetNearDistance(1.f);
	thirdPersonCamera->SetFarDistance(1000.f);
	thirdPersonCamera->SetFOV(90);

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
	std::vector<PlayerStart*> playerStartObjects = engine->GetObjectsOfType<PlayerStart>();

	Vector3 spawnPosition;
	Quaternion spawnRotation;

	if (!playerStartObjects.empty())
	{
		spawnPosition = playerStartObjects[0]->GetWorldPosition();
		spawnRotation = playerStartObjects[0]->GetWorldRotation();

		SetWorldPosition(spawnPosition);
		SetWorldRotation(spawnRotation);
	}
	else
	{
		RaycastData raycastData;
		raycastData.from = Vector3{ 0.f, 0.f, 1000.f };
		raycastData.to = Vector3{ 0.f, 0.f, -1000.f };

		RaycastSingleResult raycastResult;

		engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult);

		SetWorldPosition(raycastResult.hitPosition + Vector3{ 0.f, 0.f, 1.f });

		spawnRotation = Quaternion::FromEulerDegrees(Vector3{ 0.f, 0.f, 180.f });
		SetWorldRotation(spawnRotation);
	}

	thirdPersonCameraComponent_->SetRelativeRotation(spawnRotation);
	thirdPersonCameraComponent_->GetCamera()->SetFOV(45);

	Idle();

	cameraDistance_.Reset(defaultCameraDistance_);
	cameraDistance_.speed = 10.f;

	cameraHeightOffset_.Reset(Vector3{ 0.f, 0.f, 1.15f });
	cameraHeightOffset_.speed = 10.f;

	cameraShoulderOffset_.Reset(defaultShoulderOffset_);
	cameraShoulderOffset_.speed = 10.f;

	animationGraph_->Init();

}

void DefaultCharacter::Tick(float deltaTime)
{
	BaseCharacter::Tick(deltaTime);

	Vector2 cursorMovement = ((DefaultCharacterController*)controller_)->ConsumeCursorDeltaMoveLastFrame();
	cursorMovement.x *= mouseSensitivity_;
	cursorMovement.y *= mouseSensitivity_;

	Vector3 forwardVector = thirdPersonCameraComponent_->GetRelativeForwardVector();
	Vector3 leftVector = thirdPersonCameraComponent_->GetRelativeLeftVector();

	Vector3 newForwardVector = forwardVector.RotateVectorAroundAxis(Vector3::UpVector, cursorMovement.x);

	if ((forwardVector.z < maxCameraForwardZ_ && 0.f < cursorMovement.y) ||
		(minCameraForwardZ_ < forwardVector.z && cursorMovement.y < 0.f))
	{
		newForwardVector = newForwardVector.RotateVectorAroundAxis(leftVector, -cursorMovement.y);
	}

	thirdPersonCameraComponent_->SetRelativeRotation(newForwardVector.GetRotationNormalized());

	leftVector = thirdPersonCameraComponent_->GetRelativeLeftVector();

	cameraHeightOffset_.Interpolate(deltaTime);
	cameraShoulderOffset_.UpdateDestination(isStrafing_ ? strafingShoulderOffset_ : defaultShoulderOffset_);
	cameraShoulderOffset_.Interpolate(deltaTime);

	Vector3 rightOffsetVector = -leftVector * cameraShoulderOffset_.current;

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

		float targetDist = GoknarMath::Max(hitDistance - cameraCollisionPadding_, minCameraDistance_);
		cameraDistance_.speed = 48.f;
		cameraDistance_.UpdateDestination(targetDist);
	}
	else
	{
		cameraDistance_.speed = 10.f;
		cameraDistance_.UpdateDestination(desiredMaxDistance);
	}

	cameraDistance_.Interpolate(deltaTime);

	Vector3 newRelativePosition = (cameraHeightOffset_.current + rightOffsetVector) - (newForwardVector * cameraDistance_.current);
	thirdPersonCameraComponent_->SetRelativePosition(newRelativePosition);

	Vector3 linearVelocity = movementComponent_->GetLinearVelocity();
	Vector3 linearVelocity2D = Vector3(linearVelocity.x, linearVelocity.y, 0.f);
	float animation2DVelocityMagnitude = linearVelocity2D.Length();
	animationGraph_->SetVariable<float>("VelocityMagnitude", animation2DVelocityMagnitude);
	animationGraph_->SetVariable<float>("Speed", animation2DVelocityMagnitude);
	animationGraph_->SetVariable<bool>("IsOnGround", movementComponent_->OnGround());

	Vector3 cameraForward2D = Vector3(newForwardVector.x, newForwardVector.y, 0.f).GetNormalized();
	Vector3 cameraLeft2D = Vector3(thirdPersonCameraComponent_->GetRelativeLeftVector().x, thirdPersonCameraComponent_->GetRelativeLeftVector().y, 0.f).GetNormalized();
	float moveX = 0.f;
	float moveY = 0.f;

	if (0.0001f < animation2DVelocityMagnitude)
	{
		Vector3 movementVector = linearVelocity2D.GetNormalized();
		moveX = movementVector.Dot(cameraLeft2D) * animation2DVelocityMagnitude;
		moveY = movementVector.Dot(cameraForward2D) * animation2DVelocityMagnitude;

		float dotProduct = cameraForward2D.Dot(movementVector);
		float determinant = (cameraForward2D.x * movementVector.y) - (cameraForward2D.y * movementVector.x);
		animationGraph_->SetVariable<float>("StrafingRotation", GoknarMath::Atan2(determinant, dotProduct));
	}

	animationGraph_->SetVariable<float>("MoveX", moveX);
	animationGraph_->SetVariable<float>("MoveY", moveY);

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
		BulletHitVFXObject* newBulletHitVFXObject = new BulletHitVFXObject();
		newBulletHitVFXObject->SetWorldPosition(raycastResult.hitPosition);
		newBulletHitVFXObject->SetWorldRotation(Quaternion::FromTwoVectors(Vector3::ForwardVector, raycastResult.hitNormal));

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
		cameraHeightOffset_.UpdateDestination(Vector3{0.f, 0.f, 0.7f});
		defaultCharacterMovementComponent_->StartCrouching();
	}
	else
	{
		cameraHeightOffset_.UpdateDestination(Vector3{ 0.f, 0.f, 1.15f });

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
