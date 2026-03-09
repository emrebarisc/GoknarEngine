#include "Characters/DefaultCharacter.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"

#include "Goknar/Animation/AnimationGraph.h"
#include "Goknar/Animation/AnimationNode.h"
#include "Goknar/Animation/AnimationState.h"
#include "Goknar/Animation/AnimationNodeTransition.h"
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

	animationGraph_ = new AnimationGraph();
	animationGraph_->relativeSkeletalMeshInstance = skeletalMeshComponent_->GetMeshInstance();
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

	SetWorldPosition(raycastResult.hitPosition);

	Idle();

	cameraDistance_.Reset(defaultCameraDistance_);
	cameraDistance_.speed = 32.f;

	cameraHeightOffset_.Reset(Vector3{ 0.f, 0.f, 1.f });
	cameraHeightOffset_.speed = 8.f;

	SetupAnimationGraph();
	animationGraph_->Init();
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

	animationGraph_->SetVariable<float>("VelocityMagnitude", Vector2(movementComponent_->GetLinearVelocity()).Length());
	animationGraph_->SetVariable<bool>("IsOnGround", movementComponent_->OnGround());

	Vector3 movementVector = movementComponent_->GetMovementDirection().GetNormalized();
	float dotProduct = (forwardVector.x * movementVector.x) + (forwardVector.y * movementVector.y);
	float determinant = (forwardVector.x * movementVector.y) - (forwardVector.y * movementVector.x);
	
	animationGraph_->SetVariable<float>("StrafingRotation", GoknarMath::Atan2(determinant, dotProduct));

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

	if (isCrouched_)
	{
		cameraHeightOffset_ = Vector3{0.f, 0.f, 0.5f};
		defaultCharacterMovementComponent_->StartCrouching();

		animationGraph_->SetCurrentState(crouchState_);
	}
	else
	{
		animationGraph_->SetCurrentState(standingState_);

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

	if (isStrafing_)
	{
		if (isCrouched_)
		{

		}
		else
		{
			animationGraph_->SetCurrentState(standingStrafingState_);
		}
	}
	else
	{
		if (isCrouched_)
		{

		}
		else
		{
			animationGraph_->SetCurrentState(standingState_);
		}
	}

	if (movementComponent_)
	{
		defaultCharacterMovementComponent_->SetIsStrafing(isStrafing);
	}
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

void DefaultCharacter::SetupAnimationGraph()
{
	SetupStandingState();
	SetupCrouchState();
	SetupStandingStrafingState();

	animationGraph_->SetCurrentState(standingState_);

	animationGraph_->SetVariable<float>("VelocityMagnitude", Vector2(movementComponent_->GetLinearVelocity()).Length());
	animationGraph_->SetVariable<float>("StrafingRotation", 0.f);
	animationGraph_->SetVariable<bool>("IsOnGround", true);
	animationGraph_->SetVariable<bool>("IsJumping", false);
}

void DefaultCharacter::SetupStandingState()
{
	std::shared_ptr<AnimationNode> idleNode = std::make_shared<AnimationNode>();
	idleNode->animationName = "Armature|RifleIdle";

	std::shared_ptr<AnimationNode> walkNode = std::make_shared<AnimationNode>();
	walkNode->animationName = "Armature|RifleWalkForward";

	std::shared_ptr<AnimationNode> runNode = std::make_shared<AnimationNode>();
	runNode->animationName = "Armature|RifleSprintForward";

	std::shared_ptr<AnimationNode> onAirNode = std::make_shared<AnimationNode>();
	onAirNode->animationName = "Armature|RifleOnAir";

	std::shared_ptr<AnimationNode> jumpStartNode = std::make_shared<AnimationNode>();
	jumpStartNode->animationName = "Armature|RifleJumpStart";
	jumpStartNode->loop = false;

	std::shared_ptr<AnimationNode> jumpEndNode = std::make_shared<AnimationNode>();
	jumpEndNode->animationName = "Armature|RifleJumpFinish";
	jumpEndNode->loop = false;

	std::shared_ptr<AnimationNodeTransition> idleToWalk = std::make_shared<AnimationNodeTransition>();
	idleToWalk->targetNode = walkNode;
	idleToWalk->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	idleNode->outboundConnections.push_back(idleToWalk);

	std::shared_ptr<AnimationNodeTransition> idleToJump = std::make_shared<AnimationNodeTransition>();
	idleToJump->targetNode = jumpStartNode;
	idleToJump->conditions.push_back({ "IsJumping", CompareOp::Equal, true });
	idleNode->outboundConnections.push_back(idleToJump);

	std::shared_ptr<AnimationNodeTransition> walkToIdle = std::make_shared<AnimationNodeTransition>();
	walkToIdle->targetNode = idleNode;
	walkToIdle->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	walkNode->outboundConnections.push_back(walkToIdle);

	std::shared_ptr<AnimationNodeTransition> walkToRun = std::make_shared<AnimationNodeTransition>();
	walkToRun->targetNode = runNode;
	walkToRun->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::RUN_SPEED * 0.9f });
	walkNode->outboundConnections.push_back(walkToRun);

	std::shared_ptr<AnimationNodeTransition> walkToJump = std::make_shared<AnimationNodeTransition>();
	walkToJump->targetNode = jumpStartNode;
	walkToJump->conditions.push_back({ "IsJumping", CompareOp::Equal, true });
	walkNode->outboundConnections.push_back(walkToJump);

	std::shared_ptr<AnimationNodeTransition> runToWalk = std::make_shared<AnimationNodeTransition>();
	runToWalk->targetNode = walkNode;
	runToWalk->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::RUN_SPEED * 0.9f });
	runNode->outboundConnections.push_back(runToWalk);

	std::shared_ptr<AnimationNodeTransition> runToIdle = std::make_shared<AnimationNodeTransition>();
	runToIdle->targetNode = idleNode;
	runToIdle->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	runNode->outboundConnections.push_back(runToIdle);

	std::shared_ptr<AnimationNodeTransition> runToJump = std::make_shared<AnimationNodeTransition>();
	runToJump->targetNode = jumpStartNode;
	runToJump->conditions.push_back({ "IsJumping", CompareOp::Equal, true });
	runNode->outboundConnections.push_back(runToJump);

	std::shared_ptr<AnimationNodeTransition> jumpStartToOnAir = std::make_shared<AnimationNodeTransition>();
	jumpStartToOnAir->targetNode = onAirNode;
	jumpStartToOnAir->transitWhenAnimationDone = true;
	jumpStartNode->outboundConnections.push_back(jumpStartToOnAir);

	std::shared_ptr<AnimationNodeTransition> jumpEndToIdle = std::make_shared<AnimationNodeTransition>();
	jumpEndToIdle->targetNode = idleNode;
	jumpEndToIdle->transitWhenAnimationDone = true;
	jumpEndNode->outboundConnections.push_back(jumpEndToIdle);

	std::shared_ptr<AnimationNodeTransition> onAirToJumpEnd = std::make_shared<AnimationNodeTransition>();
	onAirToJumpEnd->targetNode = jumpEndNode;
	onAirToJumpEnd->conditions.push_back({ "IsOnGround", CompareOp::Equal, true });
	onAirNode->outboundConnections.push_back(onAirToJumpEnd);

	std::shared_ptr<AnimationNodeTransition> walkToOnAir = std::make_shared<AnimationNodeTransition>();
	walkToOnAir->targetNode = onAirNode;
	walkToOnAir->conditions.push_back({ "IsOnGround", CompareOp::Equal, false });
	walkNode->outboundConnections.push_back(walkToOnAir);

	std::shared_ptr<AnimationNodeTransition> runToOnAir = std::make_shared<AnimationNodeTransition>();
	runToOnAir->targetNode = onAirNode;
	runToOnAir->conditions.push_back({ "IsOnGround", CompareOp::Equal, false });
	runNode->outboundConnections.push_back(runToOnAir);

	standingState_ = std::make_shared<AnimationState>();
	standingState_->SetEntryNode(idleNode);
}

void DefaultCharacter::SetupCrouchState()
{
	std::shared_ptr<AnimationNode> crouchIdleNode = std::make_shared<AnimationNode>();
	crouchIdleNode->animationName = "Armature|RifleCourchIdle";

	std::shared_ptr<AnimationNode> crouchWalkForwardNode = std::make_shared<AnimationNode>();
	crouchWalkForwardNode->animationName = "Armature|RifleCrouchWalkForward";

	std::shared_ptr<AnimationNode> crouchWalkBackwardNode = std::make_shared<AnimationNode>();
	crouchWalkBackwardNode->animationName = "Armature|RifleCrouchWalkBackward";

	std::shared_ptr<AnimationNode> crouchWalkRightNode = std::make_shared<AnimationNode>();
	crouchWalkRightNode->animationName = "Armature|RifleCrouchWalkRight";

	std::shared_ptr<AnimationNode> crouchWalkLeftNode = std::make_shared<AnimationNode>();
	crouchWalkLeftNode->animationName = "Armature|RifleCrouchWalkLeft";

	std::shared_ptr<AnimationNode> crouchWalkRightForwardNode = std::make_shared<AnimationNode>();
	crouchWalkRightForwardNode->animationName = "Armature|RifleCrouchWalkRightForward";

	std::shared_ptr<AnimationNode> crouchWalkLeftForwardNode = std::make_shared<AnimationNode>();
	crouchWalkLeftForwardNode->animationName = "Armature|RifleCrouchWalkLeftForward";

	std::shared_ptr<AnimationNode> crouchWalkRightBackwardNode = std::make_shared<AnimationNode>();
	crouchWalkRightBackwardNode->animationName = "Armature|RifleCrouchWalkRightBackward";

	std::shared_ptr<AnimationNode> crouchWalkLeftBackwardNode = std::make_shared<AnimationNode>();
	crouchWalkLeftBackwardNode->animationName = "Armature|RifleCrouchWalkLeftBackward";

	std::shared_ptr<AnimationNodeTransition> crouchIdleToWalk = std::make_shared<AnimationNodeTransition>();
	crouchIdleToWalk->targetNode = crouchWalkForwardNode;
	crouchIdleToWalk->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchIdleNode->outboundConnections.push_back(crouchIdleToWalk);

	std::shared_ptr<AnimationNodeTransition> crouchWalkToIdle = std::make_shared<AnimationNodeTransition>();
	crouchWalkToIdle->targetNode = crouchIdleNode;
	crouchWalkToIdle->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchWalkForwardNode->outboundConnections.push_back(crouchWalkToIdle);

	crouchState_ = std::make_shared<AnimationState>();
	crouchState_->SetEntryNode(crouchIdleNode);
}

void DefaultCharacter::SetupStandingStrafingState()
{
	std::shared_ptr<AnimationNode> entryNode = std::make_shared<AnimationNode>();
	entryNode->animationName = "";

	std::shared_ptr<AnimationNode> walkForwardNode = std::make_shared<AnimationNode>();
	walkForwardNode->animationName = "Armature|RifleWalkForward";

	std::shared_ptr<AnimationNode> walkLeftForwardNode = std::make_shared<AnimationNode>();
	walkLeftForwardNode->animationName = "Armature|RifleWalkLeftForward";

	std::shared_ptr<AnimationNode> walkLeftNode = std::make_shared<AnimationNode>();
	walkLeftNode->animationName = "Armature|RifleWalkLeft";

	std::shared_ptr<AnimationNode> walkLeftBackwardNode = std::make_shared<AnimationNode>();
	walkLeftBackwardNode->animationName = "Armature|RifleWalkLeftBackward";

	std::shared_ptr<AnimationNode> walkBackwardNode = std::make_shared<AnimationNode>();
	walkBackwardNode->animationName = "Armature|RifleWalkBackward";

	std::shared_ptr<AnimationNode> walkRightBackwardNode = std::make_shared<AnimationNode>();
	walkRightBackwardNode->animationName = "Armature|RifleWalkRightBackward";

	std::shared_ptr<AnimationNode> walkRightNode = std::make_shared<AnimationNode>();
	walkRightNode->animationName = "Armature|RifleWalkRight";

	std::shared_ptr<AnimationNode> walkRightForwardNode = std::make_shared<AnimationNode>();
	walkRightForwardNode->animationName = "Armature|RifleWalkRightForward";

	std::shared_ptr<AnimationNodeTransition> entryToWalkForward = std::make_shared<AnimationNodeTransition>();
	entryToWalkForward->targetNode = walkForwardNode;
	entryToWalkForward->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.125f * PI });
	entryToWalkForward->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.125f * PI });
	entryNode->outboundConnections.push_back(entryToWalkForward);

	std::shared_ptr<AnimationNodeTransition> walkForwardToEntry1 = std::make_shared<AnimationNodeTransition>();
	walkForwardToEntry1->targetNode = entryNode;
	walkForwardToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.125f * PI });
	walkForwardNode->outboundConnections.push_back(walkForwardToEntry1);

	std::shared_ptr<AnimationNodeTransition> walkForwardToEntry2 = std::make_shared<AnimationNodeTransition>();
	walkForwardToEntry2->targetNode = entryNode;
	walkForwardToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.125f * PI });
	walkForwardNode->outboundConnections.push_back(walkForwardToEntry2);

	std::shared_ptr<AnimationNodeTransition> entryToWalkLeftForward = std::make_shared<AnimationNodeTransition>();
	entryToWalkLeftForward->targetNode = walkLeftForwardNode;
	entryToWalkLeftForward->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.125f * PI });
	entryToWalkLeftForward->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.375f * PI });
	entryNode->outboundConnections.push_back(entryToWalkLeftForward);

	std::shared_ptr<AnimationNodeTransition> walkLeftForwardToEntry1 = std::make_shared<AnimationNodeTransition>();
	walkLeftForwardToEntry1->targetNode = entryNode;
	walkLeftForwardToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.125f * PI });
	walkLeftForwardNode->outboundConnections.push_back(walkLeftForwardToEntry1);

	std::shared_ptr<AnimationNodeTransition> walkLeftForwardToEntry2 = std::make_shared<AnimationNodeTransition>();
	walkLeftForwardToEntry2->targetNode = entryNode;
	walkLeftForwardToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.375f * PI });
	walkLeftForwardNode->outboundConnections.push_back(walkLeftForwardToEntry2);

	std::shared_ptr<AnimationNodeTransition> entryToWalkLeft = std::make_shared<AnimationNodeTransition>();
	entryToWalkLeft->targetNode = walkLeftNode;
	entryToWalkLeft->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.375f * PI });
	entryToWalkLeft->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.625f * PI });
	entryNode->outboundConnections.push_back(entryToWalkLeft);

	std::shared_ptr<AnimationNodeTransition> walkLeftToEntry1 = std::make_shared<AnimationNodeTransition>();
	walkLeftToEntry1->targetNode = entryNode;
	walkLeftToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.375f * PI });
	walkLeftNode->outboundConnections.push_back(walkLeftToEntry1);

	std::shared_ptr<AnimationNodeTransition> walkLeftToEntry2 = std::make_shared<AnimationNodeTransition>();
	walkLeftToEntry2->targetNode = entryNode;
	walkLeftToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.625f * PI });
	walkLeftNode->outboundConnections.push_back(walkLeftToEntry2);

	std::shared_ptr<AnimationNodeTransition> entryToWalkLeftBackward = std::make_shared<AnimationNodeTransition>();
	entryToWalkLeftBackward->targetNode = walkLeftBackwardNode;
	entryToWalkLeftBackward->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.625f * PI });
	entryToWalkLeftBackward->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.875f * PI });
	entryNode->outboundConnections.push_back(entryToWalkLeftBackward);

	std::shared_ptr<AnimationNodeTransition> walkLeftBackwardToEntry1 = std::make_shared<AnimationNodeTransition>();
	walkLeftBackwardToEntry1->targetNode = entryNode;
	walkLeftBackwardToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.625f * PI });
	walkLeftBackwardNode->outboundConnections.push_back(walkLeftBackwardToEntry1);

	std::shared_ptr<AnimationNodeTransition> walkLeftBackwardToEntry2 = std::make_shared<AnimationNodeTransition>();
	walkLeftBackwardToEntry2->targetNode = entryNode;
	walkLeftBackwardToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.875f * PI });
	walkLeftBackwardNode->outboundConnections.push_back(walkLeftBackwardToEntry2);

	std::shared_ptr<AnimationNodeTransition> entryToWalkBackwardPos = std::make_shared<AnimationNodeTransition>();
	entryToWalkBackwardPos->targetNode = walkBackwardNode;
	entryToWalkBackwardPos->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.875f * PI });
	entryNode->outboundConnections.push_back(entryToWalkBackwardPos);

	std::shared_ptr<AnimationNodeTransition> entryToWalkBackwardNeg = std::make_shared<AnimationNodeTransition>();
	entryToWalkBackwardNeg->targetNode = walkBackwardNode;
	entryToWalkBackwardNeg->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.875f * PI });
	entryNode->outboundConnections.push_back(entryToWalkBackwardNeg);

	std::shared_ptr<AnimationNodeTransition> walkBackwardToEntry = std::make_shared<AnimationNodeTransition>();
	walkBackwardToEntry->targetNode = entryNode;
	walkBackwardToEntry->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.875f * PI });
	walkBackwardToEntry->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.875f * PI });
	walkBackwardNode->outboundConnections.push_back(walkBackwardToEntry);

	std::shared_ptr<AnimationNodeTransition> entryToWalkRightBackward = std::make_shared<AnimationNodeTransition>();
	entryToWalkRightBackward->targetNode = walkRightBackwardNode;
	entryToWalkRightBackward->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.875f * PI });
	entryToWalkRightBackward->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.625f * PI });
	entryNode->outboundConnections.push_back(entryToWalkRightBackward);

	std::shared_ptr<AnimationNodeTransition> walkRightBackwardToEntry1 = std::make_shared<AnimationNodeTransition>();
	walkRightBackwardToEntry1->targetNode = entryNode;
	walkRightBackwardToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.875f * PI });
	walkRightBackwardNode->outboundConnections.push_back(walkRightBackwardToEntry1);

	std::shared_ptr<AnimationNodeTransition> walkRightBackwardToEntry2 = std::make_shared<AnimationNodeTransition>();
	walkRightBackwardToEntry2->targetNode = entryNode;
	walkRightBackwardToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.625f * PI });
	walkRightBackwardNode->outboundConnections.push_back(walkRightBackwardToEntry2);

	std::shared_ptr<AnimationNodeTransition> entryToWalkRight = std::make_shared<AnimationNodeTransition>();
	entryToWalkRight->targetNode = walkRightNode;
	entryToWalkRight->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.625f * PI });
	entryToWalkRight->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.375f * PI });
	entryNode->outboundConnections.push_back(entryToWalkRight);

	std::shared_ptr<AnimationNodeTransition> walkRightToEntry1 = std::make_shared<AnimationNodeTransition>();
	walkRightToEntry1->targetNode = entryNode;
	walkRightToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.625f * PI });
	walkRightNode->outboundConnections.push_back(walkRightToEntry1);

	std::shared_ptr<AnimationNodeTransition> walkRightToEntry2 = std::make_shared<AnimationNodeTransition>();
	walkRightToEntry2->targetNode = entryNode;
	walkRightToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.375f * PI });
	walkRightNode->outboundConnections.push_back(walkRightToEntry2);

	std::shared_ptr<AnimationNodeTransition> entryToWalkRightForward = std::make_shared<AnimationNodeTransition>();
	entryToWalkRightForward->targetNode = walkRightForwardNode;
	entryToWalkRightForward->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.375f * PI });
	entryToWalkRightForward->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.125f * PI });
	entryNode->outboundConnections.push_back(entryToWalkRightForward);

	std::shared_ptr<AnimationNodeTransition> walkRightForwardToEntry1 = std::make_shared<AnimationNodeTransition>();
	walkRightForwardToEntry1->targetNode = entryNode;
	walkRightForwardToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.375f * PI });
	walkRightForwardNode->outboundConnections.push_back(walkRightForwardToEntry1);

	std::shared_ptr<AnimationNodeTransition> walkRightForwardToEntry2 = std::make_shared<AnimationNodeTransition>();
	walkRightForwardToEntry2->targetNode = entryNode;
	walkRightForwardToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.125f * PI });
	walkRightForwardNode->outboundConnections.push_back(walkRightForwardToEntry2);

	standingStrafingState_ = std::make_shared<AnimationState>();
	standingStrafingState_->SetEntryNode(entryNode);
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