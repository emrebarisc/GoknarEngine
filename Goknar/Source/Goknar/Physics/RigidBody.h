#ifndef __RIGIDBODY_H__
#define __RIGIDBODY_H__

#include "Core.h"

#include "Goknar/Physics/PhysicsObject.h"
#include "Goknar/Physics/PhysicsTypes.h"
#include "Goknar/Physics/PhysicsUtils.h"

#include "LinearMath/btVector3.h"

class btDefaultMotionState;
class btRigidBody;

struct GOKNAR_API RigidBodyInitializationData : public PhysicsObjectInitializationData
{
	btVector3 linearFactor{ PhysicsUtils::FromVector3ToBtVector3(Vector3(1.f, 1.f, 1.f)) };
	btVector3 angularFactor{ PhysicsUtils::FromVector3ToBtVector3(Vector3(1.f, 1.f, 1.f)) };

	btVector3 velocity{ PhysicsUtils::FromVector3ToBtVector3(Vector3(0.f, 0.f, 0.f)) };
	btVector3 angularVelocity{ PhysicsUtils::FromVector3ToBtVector3(Vector3(0.f, 0.f, 0.f)) };
	
	btVector3 force{ PhysicsUtils::FromVector3ToBtVector3(Vector3(0.f, 0.f, 0.f)) };
	btVector3 forcePosition{ PhysicsUtils::FromVector3ToBtVector3(Vector3(0.f, 0.f, 0.f)) };
	btVector3 centralImpulse{ PhysicsUtils::FromVector3ToBtVector3(Vector3(0.f, 0.f, 0.f)) };
	btVector3 torqueImpulse{ PhysicsUtils::FromVector3ToBtVector3(Vector3(0.f, 0.f, 0.f)) };
	btVector3 pushImpulse{ PhysicsUtils::FromVector3ToBtVector3(Vector3(0.f, 0.f, 0.f)) };
	btVector3 pushImpulsePosition{ PhysicsUtils::FromVector3ToBtVector3(Vector3(0.f, 0.f, 0.f)) };
	btVector3 torque{ PhysicsUtils::FromVector3ToBtVector3(Vector3(0.f, 0.f, 0.f)) };
	btVector3 localInertia{ PhysicsUtils::FromVector3ToBtVector3(Vector3(0.f, 0.f, 0.f)) };

	float linearSleepingThreshold{ -1.f };
	float angularSleepingThreshold{ -1.f };
};

class GOKNAR_API RigidBody : public PhysicsObject
{
public:
	RigidBody();
	virtual ~RigidBody();

	virtual void PreInit();
	virtual void Init();
	virtual void PostInit();

	virtual void Destroy() override;

	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;

	virtual void PhysicsTick(float deltaTime) override;

	virtual void SetupRigidBodyInitializationData();

	virtual void SetWorldPosition(const Vector3& worldPosition, bool updateWorldTransformationMatrix = true) override;
	virtual void SetWorldRotation(const Quaternion& worldRotation, bool updateWorldTransformationMatrix = true) override;

	virtual void SetIsActive(bool isActive) override;

	float GetMass() const
	{
		return mass_;
	}

	void SetMass(float mass);

	// For un/locking physics movement on an axis
	// For movement on all axis Vector3{1.f, 1.f, 1.f}
	// For locking on z-axis Vector3{1.f, 1.f, 0.f}
	void SetLinearFactor(const Vector3& linearFactor);

	// For un/locking physics rotation on an axis
	// For rotation on all axis Vector3{1.f, 1.f, 1.f}
	// For locking on z-axis Vector3{1.f, 1.f, 0.f}
	void SetAngularFactor(const Vector3& angularFactor);

	void SetLinearVelocity(const Vector3& velocity);
	void SetAngularVelocity(const Vector3& angularVelocity);

	void ApplyForce(const Vector3& force, const Vector3& position = Vector3::ZeroVector);
	void ApplyCentralImpulse(const Vector3& impulse);
	void ApplyTorqueImpulse(const Vector3& impulse);
	void ApplyPushImpulse(const Vector3& impulse, const Vector3& position);
	void ApplyTorque(const Vector3& torque);

	void ClearForces();
	void ClearGravity();

	void SetLinearSleepingThreshold(float linearSleepingThreshold);
	void SetAngularSleepingThreshold(float angularSleepingThreshold);

	btRigidBody* GetBulletRigidBody() const
	{
		return bulletRigidBody_;
	}

protected:
	btRigidBody* bulletRigidBody_{ nullptr };
	btDefaultMotionState* bulletMotionState_{ nullptr };

private:
	RigidBodyInitializationData* rigidBodyInitializationData_{ nullptr };
	float mass_{ 0.f };
};

#endif