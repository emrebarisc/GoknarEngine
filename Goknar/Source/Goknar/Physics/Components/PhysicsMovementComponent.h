#ifndef __PHYSICSMOVEMENTCOMPONENT_H__
#define __PHYSICSMOVEMENTCOMPONENT_H__

#include "Engine.h"
#include "Components/Component.h"
#include "Physics/PhysicsTypes.h"
#include "Physics/PhysicsWorld.h"
#include "Math/GoknarMath.h"

class btCollisionWorld;
class btKinematicCharacterController;

class OverlappingPhysicsObject;
class CollisionComponent;

struct GOKNAR_API PhysicsMovementComponentInitializationData
{
	Vector3 movementDirection{ Vector3::ZeroVector };
	Vector3 angularVelocity{ Vector3::ZeroVector };
	Vector3 linearVelocity{ Vector3::ZeroVector };
	Vector3 gravity{ engine->GetPhysicsWorld()->GetGravity() };
	Vector3 movementVelocityForGivenDuration{ Vector3::ZeroVector };
	Vector3 impulse{ Vector3::ZeroVector };
	float movementVelocityForGivenDurationDuration{ 0.f };
	float linearDamping{ 0.f };
	float angularDamping{ 0.f };
	float stepHeight{ 0.1f };
	float fallSpeed{ 55.f };
	float jumpSpeed{ 10.f };
	float maxJumpHeight{ 0.f };
	float slopeRadians{ DEGREE_TO_RADIAN(45.f) };
	float maxPenetrationDepth{ 0.2f };
	bool useGhostObjectSweepTest{ true };
	bool upInterpolate{ true };
	bool isVelocityForGivenDurationSet{ false };
	bool isMovementDirectionSet{ false };
	bool isLinearVelocitySet{ false };
	bool isAngularVelocitySet{ false };
	bool isImpulseSet{ false };
};

class GOKNAR_API PhysicsMovementComponent : public Component
{
public:
	PhysicsMovementComponent(Component* parent);
	PhysicsMovementComponent(ObjectBase* parentObjectBase);
	~PhysicsMovementComponent();

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
	virtual Vector3 GetLinearVelocity() const;

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

	virtual void SetMaxPenetrationDepth(float maxPenetrationDepth);
	virtual float GetMaxPenetrationDepth() const;

	virtual void SetUseGhostSweepTest(bool useGhostObjectSweepTest);

	virtual bool OnGround() const;
	virtual void SetUpInterpolate(bool upInterpolate);

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
	virtual void DestroyInner() override;

private:
	PhysicsMovementComponentInitializationData* initializationData_{ nullptr };

	btKinematicCharacterController* bulletKinematicCharacterController_{ nullptr };

	CollisionComponent* collisionComponent_{ nullptr };
	OverlappingPhysicsObject* ownerPhysicsObject_{ nullptr };

	float movementSpeed_{ 0.25f };
};

#endif