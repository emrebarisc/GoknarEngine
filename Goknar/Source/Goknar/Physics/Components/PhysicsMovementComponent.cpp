#include "pch.h"

#include "PhysicsMovementComponent.h"
#include "Engine.h"
#include "Physics/PhysicsUtils.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/RigidBody.h"

#include "Physics/Character.h"
#include "Physics/Components/CollisionComponent.h"
#include "Physics/Components/NonMovingTriangleMeshCollisionComponent.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"

PhysicsMovementComponent::PhysicsMovementComponent(Component* parent) :
	Component(parent)
{
	initializationData_ = new PhysicsMovementComponentInitializationData();
}

PhysicsMovementComponent::PhysicsMovementComponent(ObjectBase* parentObjectBase) :
	Component(parentObjectBase)
{
}

PhysicsMovementComponent::~PhysicsMovementComponent()
{
	delete initializationData_;
	delete bulletKinematicCharacterController_;
}

void PhysicsMovementComponent::Destroy()
{
	Component::Destroy();
}

void PhysicsMovementComponent::DestroyInner()
{
	engine->GetPhysicsWorld()->RemovePhysicsMovementComponent(this);

	delete bulletKinematicCharacterController_;
	bulletKinematicCharacterController_ = nullptr;

	Component::DestroyInner();
}

void PhysicsMovementComponent::PreInit()
{
	Component::PreInit();
}

void PhysicsMovementComponent::Init()
{
	Component::Init();

	OverlappingPhysicsObject* ownerPhysicsObject = dynamic_cast<OverlappingPhysicsObject*>(GetOwner());
	GOKNAR_CORE_ASSERT(ownerPhysicsObject, "OverlappingPhysicsObjectMovementComponent can only be added to a OverlappingPhysicsObject object");
	ownerPhysicsObject_ = ownerPhysicsObject;

	collisionComponent_ = dynamic_cast<CollisionComponent*>(ownerPhysicsObject_->GetRootComponent());

	GOKNAR_CORE_ASSERT
	(
		collisionComponent_ && 
		dynamic_cast<CollisionComponent*>(collisionComponent_) && 
		collisionComponent_->GetBulletCollisionShape()->isConvex(), 
		"Relative collision component can only be a convex CollisionComponent"
	);

	btPairCachingGhostObject* bulletPairCachingGhostObject = (btPairCachingGhostObject*)ownerPhysicsObject->GetBulletCollisionObject();

	bulletKinematicCharacterController_ =
		new btKinematicCharacterController(
			bulletPairCachingGhostObject,
			(btConvexShape*)collisionComponent_->GetBulletCollisionShape(),
			initializationData_->stepHeight,
			PhysicsUtils::FromVector3ToBtVector3(Vector3::UpVector));

	PhysicsWorld* physicsWorld = engine->GetPhysicsWorld();
	physicsWorld->AddPhysicsMovementComponent(this);

	bulletPairCachingGhostObject->setUserPointer(this);

	bulletKinematicCharacterController_->setGravity(PhysicsUtils::FromVector3ToBtVector3(initializationData_->gravity));

	if (initializationData_->isVelocityForGivenDurationSet)
	{
		bulletKinematicCharacterController_->setVelocityForTimeInterval(PhysicsUtils::FromVector3ToBtVector3(initializationData_->movementVelocityForGivenDuration), initializationData_->movementVelocityForGivenDurationDuration);
	}

	if (initializationData_->isMovementDirectionSet)
	{
		bulletKinematicCharacterController_->setWalkDirection(PhysicsUtils::FromVector3ToBtVector3(initializationData_->movementDirection * movementSpeed_));
	}

	if (initializationData_->isLinearVelocitySet)
	{
		bulletKinematicCharacterController_->setLinearVelocity(PhysicsUtils::FromVector3ToBtVector3(initializationData_->linearVelocity));
	}
	
	if (initializationData_->isAngularVelocitySet)
	{
		bulletKinematicCharacterController_->setAngularVelocity(PhysicsUtils::FromVector3ToBtVector3(initializationData_->angularVelocity));
	}
	
	if (initializationData_->isImpulseSet)
	{
		bulletKinematicCharacterController_->applyImpulse(PhysicsUtils::FromVector3ToBtVector3(initializationData_->impulse));
	}

	bulletKinematicCharacterController_->setLinearDamping(initializationData_->linearDamping);
	bulletKinematicCharacterController_->setAngularDamping(initializationData_->angularDamping);
	bulletKinematicCharacterController_->setFallSpeed(initializationData_->fallSpeed);
	bulletKinematicCharacterController_->setJumpSpeed(initializationData_->jumpSpeed);
	bulletKinematicCharacterController_->setMaxJumpHeight(initializationData_->maxJumpHeight);
	bulletKinematicCharacterController_->setMaxSlope(initializationData_->slopeRadians);
	bulletKinematicCharacterController_->setMaxPenetrationDepth(initializationData_->maxPenetrationDepth);
	bulletKinematicCharacterController_->setUseGhostSweepTest(initializationData_->useGhostObjectSweepTest);
	bulletKinematicCharacterController_->setUpInterpolate(initializationData_->upInterpolate);
	
	delete initializationData_;
	initializationData_ = nullptr;
}

void PhysicsMovementComponent::PostInit()
{
	Component::PostInit();
}

void PhysicsMovementComponent::BeginGame()
{
	Component::BeginGame();
}

void PhysicsMovementComponent::TickComponent(float deltaTime)
{
	Component::TickComponent(deltaTime);
}

void PhysicsMovementComponent::UpdateComponentToWorldTransformationMatrix()
{
	Component::UpdateComponentToWorldTransformationMatrix();
}

void PhysicsMovementComponent::SetMovementDirection(const Vector3& movementDirection)
{
	if (!GetIsInitialized())
	{
		initializationData_->isMovementDirectionSet = true;
		initializationData_->movementDirection = movementDirection;
		return;
	}

	bulletKinematicCharacterController_->setWalkDirection(PhysicsUtils::FromVector3ToBtVector3(movementDirection * movementSpeed_));
}

void PhysicsMovementComponent::SetMovementVelocityForGivenDuration(const Vector3& movementVelocity, float duration)
{
	if (!GetIsInitialized())
	{
		initializationData_->isVelocityForGivenDurationSet = true;
		initializationData_->movementVelocityForGivenDuration = movementVelocity;
		initializationData_->movementVelocityForGivenDurationDuration = duration;
		return;
	}

	bulletKinematicCharacterController_->setVelocityForTimeInterval(PhysicsUtils::FromVector3ToBtVector3(movementVelocity), duration);
}

void PhysicsMovementComponent::SetAngularVelocity(const Vector3& angularVelocity)
{
	if (!GetIsInitialized())
	{
		initializationData_->isAngularVelocitySet = true;
		initializationData_->angularVelocity = angularVelocity;
		return;
	}

	bulletKinematicCharacterController_->setAngularVelocity(PhysicsUtils::FromVector3ToBtVector3(angularVelocity));
}

Vector3 PhysicsMovementComponent::GetAngularVelocity() const
{
	return PhysicsUtils::FromBtVector3ToVector3(bulletKinematicCharacterController_->getAngularVelocity());
}

void PhysicsMovementComponent::SetLinearVelocity(const Vector3& linearVelocity)
{
	if (!GetIsInitialized())
	{
		initializationData_->isLinearVelocitySet = true;
		initializationData_->linearVelocity = linearVelocity;
		return;
	}

	bulletKinematicCharacterController_->setLinearVelocity(PhysicsUtils::FromVector3ToBtVector3(linearVelocity));
}

Vector3 PhysicsMovementComponent::GetLinearVelocity(const Vector3& linearVelocity) const
{
	return PhysicsUtils::FromBtVector3ToVector3(bulletKinematicCharacterController_->getLinearVelocity());
}

void PhysicsMovementComponent::SetLinearDamping(float linearDamping)
{
	if (!GetIsInitialized())
	{
		initializationData_->linearDamping = linearDamping;
		return;
	}

	bulletKinematicCharacterController_->setLinearDamping(linearDamping);
}

float PhysicsMovementComponent::GetLinearDamping() const
{
	return bulletKinematicCharacterController_->getLinearDamping();
}

void PhysicsMovementComponent::SetAngularDamping(float angularDamping)
{
	if (!GetIsInitialized())
	{
		initializationData_->angularDamping = angularDamping;
		return;
	}

	bulletKinematicCharacterController_->setAngularDamping(angularDamping);
}

float PhysicsMovementComponent::GetAngularDamping() const
{
	return bulletKinematicCharacterController_->getAngularDamping();
}

void PhysicsMovementComponent::Reset(PhysicsWorld* physicsWorld)
{
	bulletKinematicCharacterController_->reset(physicsWorld->GetBulletPhysicsWorld());
}

void PhysicsMovementComponent::Warp(const Vector3& origin)
{
	bulletKinematicCharacterController_->warp(PhysicsUtils::FromVector3ToBtVector3(origin));
}

void PhysicsMovementComponent::Update(PhysicsWorld* physicsWorld, float dt)
{
	Update(physicsWorld->GetBulletPhysicsWorld(), dt);
}

void PhysicsMovementComponent::Update(btCollisionWorld* bulletCollisionWorld, float dt)
{
	bulletKinematicCharacterController_->updateAction(bulletCollisionWorld, dt);
	UpdateOwnerTransformation();
}

void PhysicsMovementComponent::PreStep(PhysicsWorld* physicsWorld)
{
	PreStep(physicsWorld->GetBulletPhysicsWorld());
}

void PhysicsMovementComponent::PreStep(btCollisionWorld* bulletCollisionWorld)
{
	bulletKinematicCharacterController_->preStep(bulletCollisionWorld);
	UpdateOwnerTransformation();
}

void PhysicsMovementComponent::PlayerStep(PhysicsWorld* physicsWorld, float dt)
{
	PlayerStep(physicsWorld->GetBulletPhysicsWorld(), dt);
}

void PhysicsMovementComponent::PlayerStep(btCollisionWorld* bulletCollisionWorld, float dt)
{
	bulletKinematicCharacterController_->playerStep(bulletCollisionWorld, dt);
	UpdateOwnerTransformation();
}

void PhysicsMovementComponent::SetStepHeight(float stepHeight)
{
	if(!GetIsInitialized())
	{
		initializationData_->stepHeight = stepHeight;
		return;
	}

	bulletKinematicCharacterController_->setStepHeight(stepHeight);
}

float PhysicsMovementComponent::GetStepHeight() const
{
	if(!GetIsInitialized())
	{
		return initializationData_->stepHeight;
	}

	return bulletKinematicCharacterController_->getStepHeight();
}

void PhysicsMovementComponent::SetFallSpeed(float fallSpeed)
{
	if (!GetIsInitialized())
	{
		initializationData_->fallSpeed = fallSpeed;
		return;
	}

	bulletKinematicCharacterController_->setFallSpeed(fallSpeed);
}

float PhysicsMovementComponent::GetFallSpeed() const
{
	return bulletKinematicCharacterController_->getFallSpeed();
}

void PhysicsMovementComponent::SetJumpSpeed(float jumpSpeed)
{
	if (!GetIsInitialized())
	{
		initializationData_->jumpSpeed = jumpSpeed;
		return;
	}

	bulletKinematicCharacterController_->setJumpSpeed(jumpSpeed);
}

float PhysicsMovementComponent::GetJumpSpeed() const
{
	return bulletKinematicCharacterController_->getJumpSpeed();
}

void PhysicsMovementComponent::SetMaxJumpHeight(float maxJumpHeight)
{
	if (!GetIsInitialized())
	{
		initializationData_->maxJumpHeight = maxJumpHeight;
		return;
	}

	bulletKinematicCharacterController_->setMaxJumpHeight(maxJumpHeight);
}

bool PhysicsMovementComponent::CanJump() const
{
	return bulletKinematicCharacterController_->canJump();
}

void PhysicsMovementComponent::Jump(const Vector3& v)
{
	bulletKinematicCharacterController_->jump(PhysicsUtils::FromVector3ToBtVector3(v));
}

void PhysicsMovementComponent::ApplyImpulse(const Vector3& impulse)
{
	if (!GetIsInitialized())
	{
		initializationData_->isImpulseSet = true;
		initializationData_->impulse = impulse;
		return;
	}

	bulletKinematicCharacterController_->applyImpulse(PhysicsUtils::FromVector3ToBtVector3(impulse));
}

void PhysicsMovementComponent::SetGravity(const Vector3& gravity)
{
	if (!GetIsInitialized())
	{
		initializationData_->gravity = gravity;
		return;
	}

	bulletKinematicCharacterController_->setGravity(PhysicsUtils::FromVector3ToBtVector3(gravity));
}

Vector3 PhysicsMovementComponent::GetGravity() const
{
	return PhysicsUtils::FromBtVector3ToVector3(bulletKinematicCharacterController_->getGravity());
}

void PhysicsMovementComponent::SetMaxSlope(float slopeRadians)
{
	if (!GetIsInitialized())
	{
		initializationData_->slopeRadians = slopeRadians;
		return;
	}

	bulletKinematicCharacterController_->setMaxSlope(slopeRadians);
}

float PhysicsMovementComponent::GetMaxSlope() const
{
	return bulletKinematicCharacterController_->getMaxSlope();
}

void PhysicsMovementComponent::SetMaxPenetrationDepth(float maxPenetrationDepth)
{
	if (!GetIsInitialized())
	{
		initializationData_->maxPenetrationDepth = maxPenetrationDepth;
		return;
	}

	bulletKinematicCharacterController_->setMaxPenetrationDepth(maxPenetrationDepth);
}

float PhysicsMovementComponent::GetMaxPenetrationDepth() const
{
	return bulletKinematicCharacterController_->getMaxPenetrationDepth();
}

void PhysicsMovementComponent::SetUseGhostSweepTest(bool useGhostObjectSweepTest)
{
	if (!GetIsInitialized())
	{
		initializationData_->useGhostObjectSweepTest = useGhostObjectSweepTest;
		return;
	}

	bulletKinematicCharacterController_->setUseGhostSweepTest(useGhostObjectSweepTest);
}
bool PhysicsMovementComponent::OnGround() const
{
	return bulletKinematicCharacterController_->onGround();
}

void PhysicsMovementComponent::SetUpInterpolate(bool upInterpolate)
{
	if (!GetIsInitialized())
	{
		initializationData_->upInterpolate = upInterpolate;
		return;
	}

	bulletKinematicCharacterController_->setUpInterpolate(upInterpolate);
}

void PhysicsMovementComponent::UpdateOwnerTransformation()
{
	const btTransform& bulletWorldTransform = ownerPhysicsObject_->GetBulletCollisionObject()->getWorldTransform();

	ownerPhysicsObject_->SetWorldPosition(PhysicsUtils::FromBtVector3ToVector3(bulletWorldTransform.getOrigin()), false);
	ownerPhysicsObject_->SetWorldRotation(PhysicsUtils::FromBtQuaternionToQuaternion(bulletWorldTransform.getRotation()));
}