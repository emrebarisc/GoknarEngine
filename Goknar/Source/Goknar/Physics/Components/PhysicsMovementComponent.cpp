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
}

PhysicsMovementComponent::PhysicsMovementComponent(ObjectBase* parentObjectBase) :
	Component(parentObjectBase)
{
}

PhysicsMovementComponent::~PhysicsMovementComponent()
{
	delete bulletKinematicCharacterController_;
}

void PhysicsMovementComponent::Destroy()
{
	engine->GetPhysicsWorld()->RemovePhysicsMovementComponent(this);

	delete bulletKinematicCharacterController_;
	bulletKinematicCharacterController_ = nullptr;

	Component::Destroy();
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

	GOKNAR_CORE_ASSERT
	(
		collisionComponent_ && 
		dynamic_cast<CollisionComponent*>(collisionComponent_) && 
		collisionComponent_->GetBulletCollisionShape()->isConvex(), 
		"Relative collision component can obly be a convex CollisionComponent"
	);

	btPairCachingGhostObject* bulletPairCachingGhostObject = (btPairCachingGhostObject*)ownerPhysicsObject->GetBulletCollisionObject();

	bulletKinematicCharacterController_ =
		new btKinematicCharacterController(
			bulletPairCachingGhostObject,
			(btConvexShape*)collisionComponent_->GetBulletCollisionShape(),
			stepHeight_,
			PhysicsUtils::FromVector3ToBtVector3(Vector3::UpVector));

	PhysicsWorld* physicsWorld = engine->GetPhysicsWorld();
	physicsWorld->AddPhysicsMovementComponent(this);

	bulletPairCachingGhostObject->setUserPointer(this);

	bulletKinematicCharacterController_->setGravity(PhysicsUtils::FromVector3ToBtVector3(physicsWorld->GetGravity()));
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
	bulletKinematicCharacterController_->setWalkDirection(PhysicsUtils::FromVector3ToBtVector3(movementDirection * movementSpeed_));
}

void PhysicsMovementComponent::SetMovementVelocityForGivenDuration(const Vector3& movementVelocity, float duration)
{
	bulletKinematicCharacterController_->setVelocityForTimeInterval(PhysicsUtils::FromVector3ToBtVector3(movementVelocity), duration);
}

void PhysicsMovementComponent::SetAngularVelocity(const Vector3& angularVelocity)
{
	bulletKinematicCharacterController_->setAngularVelocity(PhysicsUtils::FromVector3ToBtVector3(angularVelocity));
}

Vector3 PhysicsMovementComponent::GetAngularVelocity() const
{
	return PhysicsUtils::FromBtVector3ToVector3(bulletKinematicCharacterController_->getAngularVelocity());
}

void PhysicsMovementComponent::SetLinearVelocity(const Vector3& linearVelocity)
{
	bulletKinematicCharacterController_->setLinearVelocity(PhysicsUtils::FromVector3ToBtVector3(linearVelocity));
}

Vector3 PhysicsMovementComponent::GetLinearVelocity(const Vector3& linearVelocity) const
{
	return PhysicsUtils::FromBtVector3ToVector3(bulletKinematicCharacterController_->getLinearVelocity());
}

void PhysicsMovementComponent::SetLinearDamping(float linearDamping)
{
	bulletKinematicCharacterController_->setLinearDamping(linearDamping);
}

float PhysicsMovementComponent::GetLinearDamping() const
{
	return bulletKinematicCharacterController_->getLinearDamping();
}

void PhysicsMovementComponent::SetAngularDamping(float angularDamping)
{
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
	stepHeight_ = stepHeight;

	if(!GetIsInitialized())
	{
		return;
	}

	bulletKinematicCharacterController_->setStepHeight(stepHeight);
}

float PhysicsMovementComponent::GetStepHeight() const
{
	if(!GetIsInitialized())
	{
		return stepHeight_;
	}

	return bulletKinematicCharacterController_->getStepHeight();
}

void PhysicsMovementComponent::SetFallSpeed(float fallSpeed)
{
	bulletKinematicCharacterController_->setFallSpeed(fallSpeed);
}

float PhysicsMovementComponent::GetFallSpeed() const
{
	return bulletKinematicCharacterController_->getFallSpeed();
}

void PhysicsMovementComponent::SetJumpSpeed(float jumpSpeed)
{
	bulletKinematicCharacterController_->setJumpSpeed(jumpSpeed);
}

float PhysicsMovementComponent::GetJumpSpeed() const
{
	return bulletKinematicCharacterController_->getJumpSpeed();
}

void PhysicsMovementComponent::SetMaxJumpHeight(float maxJumpHeight)
{
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

void PhysicsMovementComponent::ApplyImpulse(const Vector3& v)
{
	bulletKinematicCharacterController_->applyImpulse(PhysicsUtils::FromVector3ToBtVector3(v));
}

void PhysicsMovementComponent::SetGravity(const Vector3& gravity)
{
	bulletKinematicCharacterController_->setGravity(PhysicsUtils::FromVector3ToBtVector3(gravity));
}

Vector3 PhysicsMovementComponent::GetGravity() const
{
	return PhysicsUtils::FromBtVector3ToVector3(bulletKinematicCharacterController_->getGravity());
}

void PhysicsMovementComponent::SetMaxSlope(float slopeRadians)
{
	bulletKinematicCharacterController_->setMaxSlope(slopeRadians);
}

float PhysicsMovementComponent::GetMaxSlope() const
{
	return bulletKinematicCharacterController_->getMaxSlope();
}

void PhysicsMovementComponent::SetMaxPenetrationDepth(float d)
{
	bulletKinematicCharacterController_->setMaxPenetrationDepth(d);
}

float PhysicsMovementComponent::GetMaxPenetrationDepth() const
{
	return bulletKinematicCharacterController_->getMaxPenetrationDepth();
}

void PhysicsMovementComponent::SetUseGhostSweepTest(bool useGhostObjectSweepTest)
{
	bulletKinematicCharacterController_->setUseGhostSweepTest(useGhostObjectSweepTest);
}
bool PhysicsMovementComponent::OnGround() const
{
	return bulletKinematicCharacterController_->onGround();
}

void PhysicsMovementComponent::SetUpInterpolate(bool value)
{
	bulletKinematicCharacterController_->setUpInterpolate(value);
}

void PhysicsMovementComponent::UpdateOwnerTransformation()
{
	const btTransform& bulletWorldTransform = ownerPhysicsObject_->GetBulletCollisionObject()->getWorldTransform();

	ownerPhysicsObject_->SetWorldPosition(PhysicsUtils::FromBtVector3ToVector3(bulletWorldTransform.getOrigin()), false);
	ownerPhysicsObject_->SetWorldRotation(PhysicsUtils::FromBtQuaternionToQuaternion(bulletWorldTransform.getRotation()));
}