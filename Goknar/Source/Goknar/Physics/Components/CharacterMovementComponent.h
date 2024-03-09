#ifndef __CHARACTERMOVEMENTCOMPONENT_H__
#define __CHARACTERMOVEMENTCOMPONENT_H__

#include "Components/Component.h"
#include "Physics/PhysicsTypes.h"

class btCollisionWorld;
class btKinematicCharacterController;

class Character;
class CollisionComponent;
class PhysicsWorld;

class GOKNAR_API CharacterMovementComponent : public Component
{
public:
	CharacterMovementComponent(Component* parent);
	CharacterMovementComponent(ObjectBase* parentObjectBase);
	~CharacterMovementComponent();

	virtual void Destroy() override;

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	virtual void BeginGame() override;
	virtual void TickComponent(float deltaTime) override;

	virtual void UpdateComponentToWorldTransformationMatrix();

	virtual void SetMovementDirection(const Vector3& movementDirection);
	virtual void SetMovementVelocityForGivenDuration(const Vector3& movementVelocity, float duration);

	virtual void SetAngularVelocity(const Vector3& angularVelocity);
	virtual Vector3 GetAngularVelocity() const;

	virtual void SetLinearVelocity(const Vector3& linearVelocity);
	virtual Vector3 GetLinearVelocity(const Vector3& linearVelocity) const;

	virtual void SetLinearDamping(float linearDamping);
	virtual float GetLinearDamping() const;
	virtual void SetAngularDamping(float angularDamping);
	virtual float GetAngularDamping() const;

	virtual void Reset(PhysicsWorld* physicsWorld);
	virtual void Warp(const Vector3& origin);

	virtual void Update(PhysicsWorld* physicsWorld, float dt);
	virtual void Update(btCollisionWorld* bulletCollisionWorld, float dt);
	virtual void PreStep(PhysicsWorld* physicsWorld);
	virtual void PreStep(btCollisionWorld* bulletCollisionWorld);
	virtual void PlayerStep(PhysicsWorld* physicsWorld, float dt);
	virtual void PlayerStep(btCollisionWorld* bulletCollisionWorld, float dt);

	virtual void SetStepHeight(float stepHeight);
	virtual float GetStepHeight() const;
	virtual void SetFallSpeed(float fallSpeed);
	virtual float GetFallSpeed() const;
	virtual void SetJumpSpeed(float jumpSpeed);
	virtual float GetJumpSpeed() const;
	virtual void SetMaxJumpHeight(float maxJumpHeight);
	virtual bool CanJump() const;

	virtual void Jump(const Vector3& v = Vector3::ZeroVector);

	virtual void ApplyImpulse(const Vector3& v);

	virtual void SetGravity(const Vector3& gravity);
	virtual Vector3 GetGravity() const;

	virtual void SetMaxSlope(float slopeRadians);
	virtual float GetMaxSlope() const;

	virtual void SetMaxPenetrationDepth(float d);
	virtual float GetMaxPenetrationDepth() const;

	virtual void SetUseGhostSweepTest(bool useGhostObjectSweepTest);

	virtual bool OnGround() const;
	virtual void SetUpInterpolate(bool value);

	btKinematicCharacterController* GetBulletKinematicCharacterController() const
	{
		return bulletKinematicCharacterController_;
	}

	CollisionComponent* GetCollisionComponent() const
	{
		return collisionComponent_;
	}

	void SetCollisionComponent(CollisionComponent* collisionComponent)
	{
		collisionComponent_ = collisionComponent;
	}

	virtual void UpdateOwnerTransformation();

	void SetMovementSpeed(float movementSpeed)
	{
		movementSpeed_ = movementSpeed;
	}
	
	float GetMovementSpeed() const
	{
		return movementSpeed_;
	}

protected:

private:
	btKinematicCharacterController* bulletKinematicCharacterController_{ nullptr };

	CollisionComponent* collisionComponent_{ nullptr };
	Character* ownerCharacter_{ nullptr };

	float movementSpeed_{ 0.25f };
	float stepHeight_{ 0.35f };
};

#endif