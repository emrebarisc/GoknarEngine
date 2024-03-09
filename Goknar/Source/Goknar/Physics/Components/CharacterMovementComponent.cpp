#include "pch.h"

#include "CharacterMovementComponent.h"
#include "Engine.h"
#include "Physics/PhysicsUtils.h"
#include "Physics/PhysicsWorld.h"
#include "Physics/RigidBody.h"

#include "Physics/Character.h"
#include "Physics/Components/CollisionComponent.h"
#include "Physics/Components/NonMovingTriangleMeshCollisionComponent.h"

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
	delete bulletKinematicCharacterController_;
}

void CharacterMovementComponent::Destroy()
{
	engine->GetPhysicsWorld()->RemoveCharacterMovementComponent(this);

	delete bulletKinematicCharacterController_;
	bulletKinematicCharacterController_ = nullptr;

	Component::Destroy();
}

void CharacterMovementComponent::PreInit()
{
	Component::PreInit();
}

void CharacterMovementComponent::Init()
{
	Component::Init();

	Character* ownerCharacter = dynamic_cast<Character*>(GetOwner());
	GOKNAR_CORE_ASSERT(ownerCharacter, "CharacterMovementComponent can only be added to a Character object");
	ownerCharacter_ = ownerCharacter;

	GOKNAR_CORE_ASSERT
	(
		collisionComponent_ && 
		dynamic_cast<CollisionComponent*>(collisionComponent_) && 
		collisionComponent_->GetBulletCollisionShape()->isConvex(), 
		"Relative collision component can obly be a convex CollisionComponent"
	);

	btPairCachingGhostObject* bulletPairCachingGhostObject = (btPairCachingGhostObject*)ownerCharacter->GetBulletCollisionObject();

	bulletKinematicCharacterController_ =
		new btKinematicCharacterController(
			bulletPairCachingGhostObject,
			(btConvexShape*)collisionComponent_->GetBulletCollisionShape(),
			0.35f,
			PhysicsUtils::FromVector3ToBtVector3(Vector3::UpVector));

	PhysicsWorld* physicsWorld = engine->GetPhysicsWorld();
	physicsWorld->AddCharacterMovementComponent(this);

	bulletPairCachingGhostObject->setUserPointer(this);

	// bulletKinematicCharacterController_->setGravity(PhysicsUtils::FromVector3ToBtVector3(physicsWorld->GetGravity()));
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
	bulletKinematicCharacterController_->setWalkDirection(PhysicsUtils::FromVector3ToBtVector3(movementDirection * movementSpeed_));
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

void CharacterMovementComponent::Update(PhysicsWorld* physicsWorld, float dt)
{
	Update(physicsWorld->GetBulletPhysicsWorld(), dt);
}

void CharacterMovementComponent::Update(btCollisionWorld* bulletCollisionWorld, float dt)
{
	bulletKinematicCharacterController_->updateAction(bulletCollisionWorld, dt);
	UpdateOwnerTransformation();
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
	const btTransform& bulletWorldTransform = ownerCharacter_->GetBulletCollisionObject()->getWorldTransform();

	ownerCharacter_->SetWorldPosition(PhysicsUtils::FromBtVector3ToVector3(bulletWorldTransform.getOrigin()), false);
	ownerCharacter_->SetWorldRotation(PhysicsUtils::FromBtQuaternionToQuaternion(bulletWorldTransform.getRotation()));
}