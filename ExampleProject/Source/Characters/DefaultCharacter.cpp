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

	//SetupAnimationGraph();

	//standingState_->name = "Standing State";
	//standingStrafingState_->name = "Standing Strafing State";
	//crouchState_->name = "Crouch State";
	//crouchStrafingState_->name = "Crouch Strafing State";

	//animationGraph_->AddState(standingState_);
	//animationGraph_->AddState(standingStrafingState_);
	//animationGraph_->AddState(crouchState_);
	//animationGraph_->AddState(crouchStrafingState_);
	//
	//// ==========================================
	//// 1. STRAFING TRANSITIONS (Same Stance)
	//// ==========================================

	//std::shared_ptr<AnimationTransition<AnimationState>> standingToStandingStrafing = std::make_shared<AnimationTransition<AnimationState>>();
	//standingToStandingStrafing->target = standingStrafingState_; //
	//standingToStandingStrafing->conditions.push_back({ "IsCrouched", CompareOp::Equal, false }); //
	//standingToStandingStrafing->conditions.push_back({ "IsStrafing", CompareOp::Equal, true });
	//standingState_->outboundConnections.push_back(standingToStandingStrafing); //

	//std::shared_ptr<AnimationTransition<AnimationState>> standingStrafingToStanding = std::make_shared<AnimationTransition<AnimationState>>();
	//standingStrafingToStanding->target = standingState_;
	//standingStrafingToStanding->conditions.push_back({ "IsCrouched", CompareOp::Equal, false });
	//standingStrafingToStanding->conditions.push_back({ "IsStrafing", CompareOp::Equal, false });
	//standingStrafingState_->outboundConnections.push_back(standingStrafingToStanding);

	//std::shared_ptr<AnimationTransition<AnimationState>> crouchToCrouchStrafing = std::make_shared<AnimationTransition<AnimationState>>();
	//crouchToCrouchStrafing->target = crouchStrafingState_;
	//crouchToCrouchStrafing->conditions.push_back({ "IsCrouched", CompareOp::Equal, true });
	//crouchToCrouchStrafing->conditions.push_back({ "IsStrafing", CompareOp::Equal, true });
	//crouchState_->outboundConnections.push_back(crouchToCrouchStrafing);

	//std::shared_ptr<AnimationTransition<AnimationState>> crouchStrafingToCrouch = std::make_shared<AnimationTransition<AnimationState>>();
	//crouchStrafingToCrouch->target = crouchState_;
	//crouchStrafingToCrouch->conditions.push_back({ "IsCrouched", CompareOp::Equal, true });
	//crouchStrafingToCrouch->conditions.push_back({ "IsStrafing", CompareOp::Equal, false });
	//crouchStrafingState_->outboundConnections.push_back(crouchStrafingToCrouch);

	//std::shared_ptr<AnimationTransition<AnimationState>> standingToCrouch = std::make_shared<AnimationTransition<AnimationState>>();
	//standingToCrouch->target = crouchState_;
	//standingToCrouch->conditions.push_back({ "IsCrouched", CompareOp::Equal, true });
	//standingToCrouch->conditions.push_back({ "IsStrafing", CompareOp::Equal, false });
	//standingState_->outboundConnections.push_back(standingToCrouch);

	//std::shared_ptr<AnimationTransition<AnimationState>> crouchToStanding = std::make_shared<AnimationTransition<AnimationState>>();
	//crouchToStanding->target = standingState_;
	//crouchToStanding->conditions.push_back({ "IsCrouched", CompareOp::Equal, false });
	//crouchToStanding->conditions.push_back({ "IsStrafing", CompareOp::Equal, false });
	//crouchState_->outboundConnections.push_back(crouchToStanding);

	//std::shared_ptr<AnimationTransition<AnimationState>> standingStrafingToCrouchStrafing = std::make_shared<AnimationTransition<AnimationState>>();
	//standingStrafingToCrouchStrafing->target = crouchStrafingState_;
	//standingStrafingToCrouchStrafing->conditions.push_back({ "IsCrouched", CompareOp::Equal, true });
	//standingStrafingToCrouchStrafing->conditions.push_back({ "IsStrafing", CompareOp::Equal, true });
	//standingStrafingState_->outboundConnections.push_back(standingStrafingToCrouchStrafing);

	//std::shared_ptr<AnimationTransition<AnimationState>> crouchStrafingToStandingStrafing = std::make_shared<AnimationTransition<AnimationState>>();
	//crouchStrafingToStandingStrafing->target = standingStrafingState_;
	//crouchStrafingToStandingStrafing->conditions.push_back({ "IsCrouched", CompareOp::Equal, false });
	//crouchStrafingToStandingStrafing->conditions.push_back({ "IsStrafing", CompareOp::Equal, true });
	//crouchStrafingState_->outboundConnections.push_back(crouchStrafingToStandingStrafing);

	//AnimationSerializer().Serialize(animationGraph_, "Animations/AG_DefaultCharacter");

	AnimationDeserializer().Deserialize(animationGraph_, "Animations/AG_DefaultCharacter");

	//standingState_ = animationGraph_->GetStates()[0];
	//standingStrafingState_ = animationGraph_->GetStates()[1];
	//crouchState_ = animationGraph_->GetStates()[2];
	//crouchStrafingState_ = animationGraph_->GetStates()[3];

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

void DefaultCharacter::SetupAnimationGraph()
{
	SetupStandingState();
	SetupCrouchState();
	SetupStandingStrafingState();
	SetupCrouchStrafingState();

	animationGraph_->SetCurrentState(standingState_);
	animationGraph_->Init();

	animationGraph_->SetVariable<float>("VelocityMagnitude", Vector2(movementComponent_->GetLinearVelocity()).Length());
	animationGraph_->SetVariable<float>("StrafingRotation", 0.f);
	animationGraph_->SetVariable<bool>("IsOnGround", true);
	animationGraph_->SetVariable<bool>("IsJumping", false);
	animationGraph_->SetVariable<bool>("IsCrouched", false);
	animationGraph_->SetVariable<bool>("IsStrafing", false);
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

	std::shared_ptr<AnimationTransition<AnimationNode>> idleToWalk = std::make_shared<AnimationTransition<AnimationNode>>();
	idleToWalk->target = walkNode;
	idleToWalk->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	idleNode->outboundConnections.push_back(idleToWalk);

	std::shared_ptr<AnimationTransition<AnimationNode>> idleToJump = std::make_shared<AnimationTransition<AnimationNode>>();
	idleToJump->target = jumpStartNode;
	idleToJump->conditions.push_back({ "IsJumping", CompareOp::Equal, true });
	idleNode->outboundConnections.push_back(idleToJump);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkToIdle = std::make_shared<AnimationTransition<AnimationNode>>();
	walkToIdle->target = idleNode;
	walkToIdle->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	walkNode->outboundConnections.push_back(walkToIdle);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkToRun = std::make_shared<AnimationTransition<AnimationNode>>();
	walkToRun->target = runNode;
	walkToRun->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::RUN_SPEED * 0.9f });
	walkNode->outboundConnections.push_back(walkToRun);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkToJump = std::make_shared<AnimationTransition<AnimationNode>>();
	walkToJump->target = jumpStartNode;
	walkToJump->conditions.push_back({ "IsJumping", CompareOp::Equal, true });
	walkNode->outboundConnections.push_back(walkToJump);

	std::shared_ptr<AnimationTransition<AnimationNode>> runToWalk = std::make_shared<AnimationTransition<AnimationNode>>();
	runToWalk->target = walkNode;
	runToWalk->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::RUN_SPEED * 0.9f });
	runNode->outboundConnections.push_back(runToWalk);

	std::shared_ptr<AnimationTransition<AnimationNode>> runToIdle = std::make_shared<AnimationTransition<AnimationNode>>();
	runToIdle->target = idleNode;
	runToIdle->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	runNode->outboundConnections.push_back(runToIdle);

	std::shared_ptr<AnimationTransition<AnimationNode>> runToJump = std::make_shared<AnimationTransition<AnimationNode>>();
	runToJump->target = jumpStartNode;
	runToJump->conditions.push_back({ "IsJumping", CompareOp::Equal, true });
	runNode->outboundConnections.push_back(runToJump);

	std::shared_ptr<AnimationTransition<AnimationNode>> jumpStartToOnAir = std::make_shared<AnimationTransition<AnimationNode>>();
	jumpStartToOnAir->target = onAirNode;
	jumpStartToOnAir->transitWhenAnimationDone = true;
	jumpStartNode->outboundConnections.push_back(jumpStartToOnAir);

	std::shared_ptr<AnimationTransition<AnimationNode>> jumpEndToIdle = std::make_shared<AnimationTransition<AnimationNode>>();
	jumpEndToIdle->target = idleNode;
	jumpEndToIdle->transitWhenAnimationDone = true;
	jumpEndNode->outboundConnections.push_back(jumpEndToIdle);

	std::shared_ptr<AnimationTransition<AnimationNode>> onAirToJumpEnd = std::make_shared<AnimationTransition<AnimationNode>>();
	onAirToJumpEnd->target = jumpEndNode;
	onAirToJumpEnd->conditions.push_back({ "IsOnGround", CompareOp::Equal, true });
	onAirNode->outboundConnections.push_back(onAirToJumpEnd);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkToOnAir = std::make_shared<AnimationTransition<AnimationNode>>();
	walkToOnAir->target = onAirNode;
	walkToOnAir->conditions.push_back({ "IsOnGround", CompareOp::Equal, false });
	walkNode->outboundConnections.push_back(walkToOnAir);

	std::shared_ptr<AnimationTransition<AnimationNode>> runToOnAir = std::make_shared<AnimationTransition<AnimationNode>>();
	runToOnAir->target = onAirNode;
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

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchIdleToWalk = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchIdleToWalk->target = crouchWalkForwardNode;
	crouchIdleToWalk->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchIdleNode->outboundConnections.push_back(crouchIdleToWalk);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkToIdle = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkToIdle->target = crouchIdleNode;
	crouchWalkToIdle->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchWalkForwardNode->outboundConnections.push_back(crouchWalkToIdle);

	crouchState_ = std::make_shared<AnimationState>();
	crouchState_->SetEntryNode(crouchIdleNode);
}

void DefaultCharacter::SetupStandingStrafingState()
{
	std::shared_ptr<AnimationNode> idleNode = std::make_shared<AnimationNode>();
	idleNode->animationName = "Armature|RifleIdle";

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

	// --- Forward ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkForward = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkForward->target = walkForwardNode;
	entryToWalkForward->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.125f * PI });
	entryToWalkForward->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.125f * PI });
	entryToWalkForward->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	idleNode->outboundConnections.push_back(entryToWalkForward);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkForwardToEntry1 = std::make_shared<AnimationTransition<AnimationNode>>();
	walkForwardToEntry1->target = idleNode;
	walkForwardToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.125f * PI });
	walkForwardNode->outboundConnections.push_back(walkForwardToEntry1);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkForwardToEntry2 = std::make_shared<AnimationTransition<AnimationNode>>();
	walkForwardToEntry2->target = idleNode;
	walkForwardToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.125f * PI });
	walkForwardNode->outboundConnections.push_back(walkForwardToEntry2);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkForwardToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	walkForwardToIdleStop->target = idleNode;
	walkForwardToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	walkForwardNode->outboundConnections.push_back(walkForwardToIdleStop);

	// --- Left Forward ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkLeftForward = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkLeftForward->target = walkLeftForwardNode;
	entryToWalkLeftForward->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.125f * PI });
	entryToWalkLeftForward->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.375f * PI });
	entryToWalkLeftForward->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	idleNode->outboundConnections.push_back(entryToWalkLeftForward);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkLeftForwardToEntry1 = std::make_shared<AnimationTransition<AnimationNode>>();
	walkLeftForwardToEntry1->target = idleNode;
	walkLeftForwardToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.125f * PI });
	walkLeftForwardNode->outboundConnections.push_back(walkLeftForwardToEntry1);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkLeftForwardToEntry2 = std::make_shared<AnimationTransition<AnimationNode>>();
	walkLeftForwardToEntry2->target = idleNode;
	walkLeftForwardToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.375f * PI });
	walkLeftForwardNode->outboundConnections.push_back(walkLeftForwardToEntry2);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkLeftForwardToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	walkLeftForwardToIdleStop->target = idleNode;
	walkLeftForwardToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	walkLeftForwardNode->outboundConnections.push_back(walkLeftForwardToIdleStop);

	// --- Left ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkLeft = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkLeft->target = walkLeftNode;
	entryToWalkLeft->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.375f * PI });
	entryToWalkLeft->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.625f * PI });
	entryToWalkLeft->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	idleNode->outboundConnections.push_back(entryToWalkLeft);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkLeftToEntry1 = std::make_shared<AnimationTransition<AnimationNode>>();
	walkLeftToEntry1->target = idleNode;
	walkLeftToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.375f * PI });
	walkLeftNode->outboundConnections.push_back(walkLeftToEntry1);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkLeftToEntry2 = std::make_shared<AnimationTransition<AnimationNode>>();
	walkLeftToEntry2->target = idleNode;
	walkLeftToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.625f * PI });
	walkLeftNode->outboundConnections.push_back(walkLeftToEntry2);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkLeftToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	walkLeftToIdleStop->target = idleNode;
	walkLeftToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	walkLeftNode->outboundConnections.push_back(walkLeftToIdleStop);

	// --- Left Backward ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkLeftBackward = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkLeftBackward->target = walkLeftBackwardNode;
	entryToWalkLeftBackward->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.625f * PI });
	entryToWalkLeftBackward->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.875f * PI });
	entryToWalkLeftBackward->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	idleNode->outboundConnections.push_back(entryToWalkLeftBackward);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkLeftBackwardToEntry1 = std::make_shared<AnimationTransition<AnimationNode>>();
	walkLeftBackwardToEntry1->target = idleNode;
	walkLeftBackwardToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.625f * PI });
	walkLeftBackwardNode->outboundConnections.push_back(walkLeftBackwardToEntry1);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkLeftBackwardToEntry2 = std::make_shared<AnimationTransition<AnimationNode>>();
	walkLeftBackwardToEntry2->target = idleNode;
	walkLeftBackwardToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.875f * PI });
	walkLeftBackwardNode->outboundConnections.push_back(walkLeftBackwardToEntry2);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkLeftBackwardToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	walkLeftBackwardToIdleStop->target = idleNode;
	walkLeftBackwardToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	walkLeftBackwardNode->outboundConnections.push_back(walkLeftBackwardToIdleStop);

	// --- Backward ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkBackwardPos = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkBackwardPos->target = walkBackwardNode;
	entryToWalkBackwardPos->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.875f * PI });
	entryToWalkBackwardPos->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	idleNode->outboundConnections.push_back(entryToWalkBackwardPos);

	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkBackwardNeg = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkBackwardNeg->target = walkBackwardNode;
	entryToWalkBackwardNeg->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.875f * PI });
	entryToWalkBackwardNeg->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	idleNode->outboundConnections.push_back(entryToWalkBackwardNeg);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkBackwardToEntry = std::make_shared<AnimationTransition<AnimationNode>>();
	walkBackwardToEntry->target = idleNode;
	walkBackwardToEntry->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.875f * PI });
	walkBackwardToEntry->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.875f * PI });
	walkBackwardNode->outboundConnections.push_back(walkBackwardToEntry);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkBackwardToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	walkBackwardToIdleStop->target = idleNode;
	walkBackwardToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	walkBackwardNode->outboundConnections.push_back(walkBackwardToIdleStop);

	// --- Right Backward ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkRightBackward = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkRightBackward->target = walkRightBackwardNode;
	entryToWalkRightBackward->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.875f * PI });
	entryToWalkRightBackward->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.625f * PI });
	entryToWalkRightBackward->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	idleNode->outboundConnections.push_back(entryToWalkRightBackward);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkRightBackwardToEntry1 = std::make_shared<AnimationTransition<AnimationNode>>();
	walkRightBackwardToEntry1->target = idleNode;
	walkRightBackwardToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.875f * PI });
	walkRightBackwardNode->outboundConnections.push_back(walkRightBackwardToEntry1);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkRightBackwardToEntry2 = std::make_shared<AnimationTransition<AnimationNode>>();
	walkRightBackwardToEntry2->target = idleNode;
	walkRightBackwardToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.625f * PI });
	walkRightBackwardNode->outboundConnections.push_back(walkRightBackwardToEntry2);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkRightBackwardToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	walkRightBackwardToIdleStop->target = idleNode;
	walkRightBackwardToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	walkRightBackwardNode->outboundConnections.push_back(walkRightBackwardToIdleStop);

	// --- Right ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkRight = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkRight->target = walkRightNode;
	entryToWalkRight->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.625f * PI });
	entryToWalkRight->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.375f * PI });
	entryToWalkRight->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	idleNode->outboundConnections.push_back(entryToWalkRight);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkRightToEntry1 = std::make_shared<AnimationTransition<AnimationNode>>();
	walkRightToEntry1->target = idleNode;
	walkRightToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.625f * PI });
	walkRightNode->outboundConnections.push_back(walkRightToEntry1);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkRightToEntry2 = std::make_shared<AnimationTransition<AnimationNode>>();
	walkRightToEntry2->target = idleNode;
	walkRightToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.375f * PI });
	walkRightNode->outboundConnections.push_back(walkRightToEntry2);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkRightToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	walkRightToIdleStop->target = idleNode;
	walkRightToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	walkRightNode->outboundConnections.push_back(walkRightToIdleStop);

	// --- Right Forward ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkRightForward = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkRightForward->target = walkRightForwardNode;
	entryToWalkRightForward->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.375f * PI });
	entryToWalkRightForward->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.125f * PI });
	entryToWalkRightForward->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	idleNode->outboundConnections.push_back(entryToWalkRightForward);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkRightForwardToEntry1 = std::make_shared<AnimationTransition<AnimationNode>>();
	walkRightForwardToEntry1->target = idleNode;
	walkRightForwardToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.375f * PI });
	walkRightForwardNode->outboundConnections.push_back(walkRightForwardToEntry1);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkRightForwardToEntry2 = std::make_shared<AnimationTransition<AnimationNode>>();
	walkRightForwardToEntry2->target = idleNode;
	walkRightForwardToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.125f * PI });
	walkRightForwardNode->outboundConnections.push_back(walkRightForwardToEntry2);

	std::shared_ptr<AnimationTransition<AnimationNode>> walkRightForwardToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	walkRightForwardToIdleStop->target = idleNode;
	walkRightForwardToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	walkRightForwardNode->outboundConnections.push_back(walkRightForwardToIdleStop);

	standingStrafingState_ = std::make_shared<AnimationState>();
	standingStrafingState_->SetEntryNode(idleNode);
}

void DefaultCharacter::SetupCrouchStrafingState()
{
	std::shared_ptr<AnimationNode> entryNode = std::make_shared<AnimationNode>();
	entryNode->animationName = "";

	std::shared_ptr<AnimationNode> crouchIdleNode = std::make_shared<AnimationNode>();
	crouchIdleNode->animationName = "Armature|RifleCourchIdle"; // Fixed node name assignment here

	std::shared_ptr<AnimationNode> crouchWalkForwardNode = std::make_shared<AnimationNode>();
	crouchWalkForwardNode->animationName = "Armature|RifleCrouchWalkForward";

	std::shared_ptr<AnimationNode> crouchWalkLeftForwardNode = std::make_shared<AnimationNode>();
	crouchWalkLeftForwardNode->animationName = "Armature|RifleCrouchWalkLeftForward";

	std::shared_ptr<AnimationNode> crouchWalkLeftNode = std::make_shared<AnimationNode>();
	crouchWalkLeftNode->animationName = "Armature|RifleCrouchWalkLeft";

	std::shared_ptr<AnimationNode> crouchWalkLeftBackwardNode = std::make_shared<AnimationNode>();
	crouchWalkLeftBackwardNode->animationName = "Armature|RifleCrouchWalkLeftBackward";

	std::shared_ptr<AnimationNode> crouchWalkBackwardNode = std::make_shared<AnimationNode>();
	crouchWalkBackwardNode->animationName = "Armature|RifleCrouchWalkBackward";

	std::shared_ptr<AnimationNode> crouchWalkRightBackwardNode = std::make_shared<AnimationNode>();
	crouchWalkRightBackwardNode->animationName = "Armature|RifleCrouchWalkRightBackward";

	std::shared_ptr<AnimationNode> crouchWalkRightNode = std::make_shared<AnimationNode>();
	crouchWalkRightNode->animationName = "Armature|RifleCrouchWalkRight";

	std::shared_ptr<AnimationNode> crouchWalkRightForwardNode = std::make_shared<AnimationNode>();
	crouchWalkRightForwardNode->animationName = "Armature|RifleCrouchWalkRightForward";

	// --- Forward ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkForward = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkForward->target = crouchWalkForwardNode;
	entryToWalkForward->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.125f * PI });
	entryToWalkForward->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.125f * PI });
	entryToWalkForward->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchIdleNode->outboundConnections.push_back(entryToWalkForward);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkForwardToEntry1 = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkForwardToEntry1->target = crouchIdleNode;
	crouchWalkForwardToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.125f * PI });
	crouchWalkForwardNode->outboundConnections.push_back(crouchWalkForwardToEntry1);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkForwardToEntry2 = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkForwardToEntry2->target = crouchIdleNode;
	crouchWalkForwardToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.125f * PI });
	crouchWalkForwardNode->outboundConnections.push_back(crouchWalkForwardToEntry2);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkForwardToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkForwardToIdleStop->target = crouchIdleNode;
	crouchWalkForwardToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchWalkForwardNode->outboundConnections.push_back(crouchWalkForwardToIdleStop);

	// --- Left Forward ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkLeftForward = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkLeftForward->target = crouchWalkLeftForwardNode;
	entryToWalkLeftForward->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.125f * PI });
	entryToWalkLeftForward->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.375f * PI });
	entryToWalkLeftForward->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchIdleNode->outboundConnections.push_back(entryToWalkLeftForward);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkLeftForwardToEntry1 = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkLeftForwardToEntry1->target = crouchIdleNode;
	crouchWalkLeftForwardToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.125f * PI });
	crouchWalkLeftForwardNode->outboundConnections.push_back(crouchWalkLeftForwardToEntry1);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkLeftForwardToEntry2 = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkLeftForwardToEntry2->target = crouchIdleNode;
	crouchWalkLeftForwardToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.375f * PI });
	crouchWalkLeftForwardNode->outboundConnections.push_back(crouchWalkLeftForwardToEntry2);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkLeftForwardToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkLeftForwardToIdleStop->target = crouchIdleNode;
	crouchWalkLeftForwardToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchWalkLeftForwardNode->outboundConnections.push_back(crouchWalkLeftForwardToIdleStop);

	// --- Left ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkLeft = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkLeft->target = crouchWalkLeftNode;
	entryToWalkLeft->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.375f * PI });
	entryToWalkLeft->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.625f * PI });
	entryToWalkLeft->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchIdleNode->outboundConnections.push_back(entryToWalkLeft);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkLeftToEntry1 = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkLeftToEntry1->target = crouchIdleNode;
	crouchWalkLeftToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.375f * PI });
	crouchWalkLeftNode->outboundConnections.push_back(crouchWalkLeftToEntry1);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkLeftToEntry2 = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkLeftToEntry2->target = crouchIdleNode;
	crouchWalkLeftToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.625f * PI });
	crouchWalkLeftNode->outboundConnections.push_back(crouchWalkLeftToEntry2);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkLeftToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkLeftToIdleStop->target = crouchIdleNode;
	crouchWalkLeftToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchWalkLeftNode->outboundConnections.push_back(crouchWalkLeftToIdleStop);

	// --- Left Backward ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkLeftBackward = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkLeftBackward->target = crouchWalkLeftBackwardNode;
	entryToWalkLeftBackward->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.625f * PI });
	entryToWalkLeftBackward->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.875f * PI });
	entryToWalkLeftBackward->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchIdleNode->outboundConnections.push_back(entryToWalkLeftBackward);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkLeftBackwardToEntry1 = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkLeftBackwardToEntry1->target = crouchIdleNode;
	crouchWalkLeftBackwardToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.625f * PI });
	crouchWalkLeftBackwardNode->outboundConnections.push_back(crouchWalkLeftBackwardToEntry1);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkLeftBackwardToEntry2 = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkLeftBackwardToEntry2->target = crouchIdleNode;
	crouchWalkLeftBackwardToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.875f * PI });
	crouchWalkLeftBackwardNode->outboundConnections.push_back(crouchWalkLeftBackwardToEntry2);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkLeftBackwardToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkLeftBackwardToIdleStop->target = crouchIdleNode;
	crouchWalkLeftBackwardToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchWalkLeftBackwardNode->outboundConnections.push_back(crouchWalkLeftBackwardToIdleStop);

	// --- Backward ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkBackwardPos = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkBackwardPos->target = crouchWalkBackwardNode;
	entryToWalkBackwardPos->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, 0.875f * PI });
	entryToWalkBackwardPos->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchIdleNode->outboundConnections.push_back(entryToWalkBackwardPos);

	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkBackwardNeg = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkBackwardNeg->target = crouchWalkBackwardNode;
	entryToWalkBackwardNeg->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.875f * PI });
	entryToWalkBackwardNeg->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchIdleNode->outboundConnections.push_back(entryToWalkBackwardNeg);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkBackwardToEntry = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkBackwardToEntry->target = crouchIdleNode;
	crouchWalkBackwardToEntry->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.875f * PI });
	crouchWalkBackwardToEntry->conditions.push_back({ "StrafingRotation", CompareOp::Less, 0.875f * PI });
	crouchWalkBackwardNode->outboundConnections.push_back(crouchWalkBackwardToEntry);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkBackwardToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkBackwardToIdleStop->target = crouchIdleNode;
	crouchWalkBackwardToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchWalkBackwardNode->outboundConnections.push_back(crouchWalkBackwardToIdleStop);

	// --- Right Backward ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkRightBackward = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkRightBackward->target = crouchWalkRightBackwardNode;
	entryToWalkRightBackward->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.875f * PI });
	entryToWalkRightBackward->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.625f * PI });
	entryToWalkRightBackward->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchIdleNode->outboundConnections.push_back(entryToWalkRightBackward);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkRightBackwardToEntry1 = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkRightBackwardToEntry1->target = crouchIdleNode;
	crouchWalkRightBackwardToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.875f * PI });
	crouchWalkRightBackwardNode->outboundConnections.push_back(crouchWalkRightBackwardToEntry1);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkRightBackwardToEntry2 = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkRightBackwardToEntry2->target = crouchIdleNode;
	crouchWalkRightBackwardToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.625f * PI });
	crouchWalkRightBackwardNode->outboundConnections.push_back(crouchWalkRightBackwardToEntry2);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkRightBackwardToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkRightBackwardToIdleStop->target = crouchIdleNode;
	crouchWalkRightBackwardToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchWalkRightBackwardNode->outboundConnections.push_back(crouchWalkRightBackwardToIdleStop);

	// --- Right ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkRight = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkRight->target = crouchWalkRightNode;
	entryToWalkRight->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.625f * PI });
	entryToWalkRight->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.375f * PI });
	entryToWalkRight->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchIdleNode->outboundConnections.push_back(entryToWalkRight);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkRightToEntry1 = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkRightToEntry1->target = crouchIdleNode;
	crouchWalkRightToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.625f * PI });
	crouchWalkRightNode->outboundConnections.push_back(crouchWalkRightToEntry1);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkRightToEntry2 = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkRightToEntry2->target = crouchIdleNode;
	crouchWalkRightToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.375f * PI });
	crouchWalkRightNode->outboundConnections.push_back(crouchWalkRightToEntry2);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkRightToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkRightToIdleStop->target = crouchIdleNode;
	crouchWalkRightToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchWalkRightNode->outboundConnections.push_back(crouchWalkRightToIdleStop);

	// --- Right Forward ---
	std::shared_ptr<AnimationTransition<AnimationNode>> entryToWalkRightForward = std::make_shared<AnimationTransition<AnimationNode>>();
	entryToWalkRightForward->target = crouchWalkRightForwardNode;
	entryToWalkRightForward->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.375f * PI });
	entryToWalkRightForward->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.125f * PI });
	entryToWalkRightForward->conditions.push_back({ "VelocityMagnitude", CompareOp::GreaterOrEqual, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchIdleNode->outboundConnections.push_back(entryToWalkRightForward);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkRightForwardToEntry1 = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkRightForwardToEntry1->target = crouchIdleNode;
	crouchWalkRightForwardToEntry1->conditions.push_back({ "StrafingRotation", CompareOp::Less, -0.375f * PI });
	crouchWalkRightForwardNode->outboundConnections.push_back(crouchWalkRightForwardToEntry1);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkRightForwardToEntry2 = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkRightForwardToEntry2->target = crouchIdleNode;
	crouchWalkRightForwardToEntry2->conditions.push_back({ "StrafingRotation", CompareOp::GreaterOrEqual, -0.125f * PI });
	crouchWalkRightForwardNode->outboundConnections.push_back(crouchWalkRightForwardToEntry2);

	std::shared_ptr<AnimationTransition<AnimationNode>> crouchWalkRightForwardToIdleStop = std::make_shared<AnimationTransition<AnimationNode>>();
	crouchWalkRightForwardToIdleStop->target = crouchIdleNode;
	crouchWalkRightForwardToIdleStop->conditions.push_back({ "VelocityMagnitude", CompareOp::Less, DefaultCharacterMovementComponent::MIN_SPEED });
	crouchWalkRightForwardNode->outboundConnections.push_back(crouchWalkRightForwardToIdleStop);

	crouchStrafingState_ = std::make_shared<AnimationState>();
	crouchStrafingState_->SetEntryNode(crouchIdleNode);
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