#include "pch.h"

#include "CharacterMovementComponent.h"
#include "Engine.h"
#include "Physics/PhysicsUtils.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/RigidBody.h"

#include "Physics/OverlappingPhysicsObject.h"
#include "Physics/Components/CollisionComponent.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"

CharacterMovementComponent::CharacterMovementComponent(Component* parent) :
	Component(parent)
{
}

CharacterMovementComponent::CharacterMovementComponent(ObjectBase* parentObjectBase) :
	Component(parentObjectBase)
{
}

CharacterMovementComponent::~CharacterMovementComponent()
{
}

void CharacterMovementComponent::Destroy()
{
	Component::Destroy();
	
	engine->GetPhysicsWorld()->RemoveCharacterMovementComponent(this);
}

void CharacterMovementComponent::PreInit()
{
	Component::PreInit();
}

void CharacterMovementComponent::Init()
{
	Component::Init();

	ownerOverlappingPhysicsObject_ = dynamic_cast<OverlappingPhysicsObject*>(GetOwner());
	GOKNAR_CORE_ASSERT(ownerOverlappingPhysicsObject_, "CHARACTER MOVEMENT OWNER CAN ONLY BE AN OBJECT OF OverlappingPhysicsObject!");

	btPairCachingGhostObject* bulletPairCachingGhostObject = dynamic_cast<btPairCachingGhostObject*>(ownerOverlappingPhysicsObject_->GetBulletCollisionObject());
	GOKNAR_CORE_ASSERT(bulletPairCachingGhostObject, "CHARACTER MOVEMENT OWNER CAN ONLY HAVE A BULLET OBJECT OF btPairCachingGhostObject!");

	CollisionComponent* ownerCollisionComponent = dynamic_cast<CollisionComponent*>(ownerOverlappingPhysicsObject_->GetRootComponent());
	GOKNAR_CORE_ASSERT(ownerCollisionComponent, "CHARACTER MOVEMENT OWNER'S ROOT COMPONENT CAN ONLY BE A COMPONENT OF CollisionComponent!");

	btConvexShape* bulletConvexShape = dynamic_cast<btConvexShape*>(ownerCollisionComponent->GetBulletCollisionShape());
	GOKNAR_CORE_ASSERT(bulletConvexShape, "CHARACTER MOVEMENT OWNER'S ROOT COMPONENT CAN ONLY HAVE COMPONENT OF btConvexShape!");

	bulletKinematicCharacterController_ = 
		new btKinematicCharacterController(
			bulletPairCachingGhostObject, 
			bulletConvexShape,
			0.25f,
			PhysicsUtils::FromVector3ToBtVector3(Vector3::UpVector));

	SetGravity(engine->GetPhysicsWorld()->GetGravity());

	engine->GetPhysicsWorld()->AddCharacterMovementComponent(this);
}

void CharacterMovementComponent::PostInit()
{
	Component::PostInit();
}
	
void CharacterMovementComponent::BeginGame()
{
	Component::BeginGame();
}

void CharacterMovementComponent::TickComponent(float deltaTime)
{
	Component::TickComponent(deltaTime);
}

void CharacterMovementComponent::UpdateComponentToWorldTransformationMatrix()
{
	Component::UpdateComponentToWorldTransformationMatrix();
}

void CharacterMovementComponent::SetMovementDirection(const Vector3& movementDirection)
{
	bulletKinematicCharacterController_->setWalkDirection(PhysicsUtils::FromVector3ToBtVector3(movementDirection));
}

void CharacterMovementComponent::SetMovementVelocityForGivenDuration(const Vector3& movementVelocity, float duration)
{
	bulletKinematicCharacterController_->setVelocityForTimeInterval(PhysicsUtils::FromVector3ToBtVector3(movementVelocity), duration);
}

void CharacterMovementComponent::SetAngularVelocity(const Vector3& angularVelocity)
{
	bulletKinematicCharacterController_->setAngularVelocity(PhysicsUtils::FromVector3ToBtVector3(angularVelocity));
}

Vector3 CharacterMovementComponent::GetAngularVelocity() const
{
	return PhysicsUtils::FromBtVector3ToVector3(bulletKinematicCharacterController_->getAngularVelocity());
}

void CharacterMovementComponent::SetLinearVelocity(const Vector3& linearVelocity)
{
	bulletKinematicCharacterController_->setLinearVelocity(PhysicsUtils::FromVector3ToBtVector3(linearVelocity));
}

Vector3 CharacterMovementComponent::GetLinearVelocity(const Vector3& linearVelocity) const
{
	return PhysicsUtils::FromBtVector3ToVector3(bulletKinematicCharacterController_->getLinearVelocity());
}

void CharacterMovementComponent::SetLinearDamping(float linearDamping)
{
	bulletKinematicCharacterController_->setLinearDamping(linearDamping);
}

float CharacterMovementComponent::GetLinearDamping() const
{
	return bulletKinematicCharacterController_->getLinearDamping();
}

void CharacterMovementComponent::SetAngularDamping(float angularDamping)
{
	bulletKinematicCharacterController_->setAngularDamping(angularDamping);
}

float CharacterMovementComponent::GetAngularDamping() const
{
	return bulletKinematicCharacterController_->getAngularDamping();
}

void CharacterMovementComponent::Reset(PhysicsWorld* physicsWorld)
{
	bulletKinematicCharacterController_->reset(physicsWorld->GetBulletPhysicsWorld());
}

void CharacterMovementComponent::Warp(const Vector3& origin)
{
	bulletKinematicCharacterController_->warp(PhysicsUtils::FromVector3ToBtVector3(origin));
}

void CharacterMovementComponent::PreStep(PhysicsWorld* physicsWorld)
{
	PreStep(physicsWorld->GetBulletPhysicsWorld());
}

void CharacterMovementComponent::PreStep(btCollisionWorld* bulletCollisionWorld)
{
	bulletKinematicCharacterController_->preStep(bulletCollisionWorld);
	UpdateOwnerTransformation();
}

void CharacterMovementComponent::PlayerStep(PhysicsWorld* physicsWorld, float dt)
{
	PlayerStep(physicsWorld->GetBulletPhysicsWorld(), dt);
}

void CharacterMovementComponent::PlayerStep(btCollisionWorld* bulletCollisionWorld, float dt)
{
	bulletKinematicCharacterController_->playerStep(bulletCollisionWorld, dt);
	UpdateOwnerTransformation();
}

void CharacterMovementComponent::SetStepHeight(float h)
{
	bulletKinematicCharacterController_->setStepHeight(h);
}

float CharacterMovementComponent::GetStepHeight() const
{
	return bulletKinematicCharacterController_->getStepHeight();
}

void CharacterMovementComponent::SetFallSpeed(float fallSpeed)
{
	bulletKinematicCharacterController_->setFallSpeed(fallSpeed);
}

float CharacterMovementComponent::GetFallSpeed() const
{
	return bulletKinematicCharacterController_->getFallSpeed();
}

void CharacterMovementComponent::SetJumpSpeed(float jumpSpeed)
{
	bulletKinematicCharacterController_->setJumpSpeed(jumpSpeed);
}

float CharacterMovementComponent::GetJumpSpeed() const
{
	return bulletKinematicCharacterController_->getJumpSpeed();
}

void CharacterMovementComponent::SetMaxJumpHeight(float maxJumpHeight)
{
	bulletKinematicCharacterController_->setMaxJumpHeight(maxJumpHeight);
}

bool CharacterMovementComponent::CanJump() const
{
	return bulletKinematicCharacterController_->canJump();
}

void CharacterMovementComponent::Jump(const Vector3& v)
{
	bulletKinematicCharacterController_->jump(PhysicsUtils::FromVector3ToBtVector3(v));
}

void CharacterMovementComponent::ApplyImpulse(const Vector3& v)
{
	bulletKinematicCharacterController_->applyImpulse(PhysicsUtils::FromVector3ToBtVector3(v));
}

void CharacterMovementComponent::SetGravity(const Vector3& gravity)
{
	bulletKinematicCharacterController_->setGravity(PhysicsUtils::FromVector3ToBtVector3(gravity));
}

Vector3 CharacterMovementComponent::GetGravity() const
{
	return PhysicsUtils::FromBtVector3ToVector3(bulletKinematicCharacterController_->getGravity());
}

void CharacterMovementComponent::SetMaxSlope(float slopeRadians)
{
	bulletKinematicCharacterController_->setMaxSlope(slopeRadians);
}

float CharacterMovementComponent::GetMaxSlope() const
{
	return bulletKinematicCharacterController_->getMaxSlope();
}

void CharacterMovementComponent::SetMaxPenetrationDepth(float d)
{
	bulletKinematicCharacterController_->setMaxPenetrationDepth(d);
}

float CharacterMovementComponent::GetMaxPenetrationDepth() const
{
	return bulletKinematicCharacterController_->getMaxPenetrationDepth();
}

void CharacterMovementComponent::SetUseGhostSweepTest(bool useGhostObjectSweepTest)
{
	bulletKinematicCharacterController_->setUseGhostSweepTest(useGhostObjectSweepTest);
}
bool CharacterMovementComponent::OnGround() const
{
	return bulletKinematicCharacterController_->onGround();
}

void CharacterMovementComponent::SetUpInterpolate(bool value)
{
	bulletKinematicCharacterController_->setUpInterpolate(value);
}

void CharacterMovementComponent::UpdateOwnerTransformation()
{
    const btTransform& bulletWorldTransform = ownerOverlappingPhysicsObject_->GetBulletCollisionObject()->getWorldTransform();

    ownerOverlappingPhysicsObject_->SetWorldPosition(PhysicsUtils::FromBtVector3ToVector3(bulletWorldTransform.getOrigin()), false);
    ownerOverlappingPhysicsObject_->SetWorldRotation(PhysicsUtils::FromBtQuaternionToQuaternion(bulletWorldTransform.getRotation()));
}