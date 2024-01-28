#ifndef __RIGIDBODY_H__
#define __RIGIDBODY_H__

#include "Core.h"

#include "Goknar/ObjectBase.h"

#include "LinearMath/btVector3.h"

class btRigidBody;

struct GOKNAR_API RigidBodyInitializationData
{
	btVector3 linearFactor{ btVector3(1.f, 1.f, 1.f) };
	btVector3 angularFactor{ btVector3(1.f, 1.f, 1.f) };

	btVector3 velocity{ btVector3(0.f, 0.f, 0.f) };
	btVector3 angularVelocity{ btVector3(0.f, 0.f, 0.f) };
	
	btVector3 force{ btVector3(0.f, 0.f, 0.f) };
	btVector3 forcePosition{ btVector3(0.f, 0.f, 0.f) };
	btVector3 centralImpulse{ btVector3(0.f, 0.f, 0.f) };
	btVector3 torqueImpulse{ btVector3(0.f, 0.f, 0.f) };
	btVector3 pushImpulse{ btVector3(0.f, 0.f, 0.f) };
	btVector3 pushImpulsePosition{ btVector3(0.f, 0.f, 0.f) };
	btVector3 localInertia{ btVector3(0.f, 0.f, 0.f) };
};

class GOKNAR_API RigidBody : public ObjectBase
{
public:
	RigidBody();
	~RigidBody();

	virtual void PreInit();
	virtual void Init();
	virtual void PostInit();

	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;

	virtual void PhysicsTick(float deltaTime);

	virtual void SetWorldPosition(const Vector3& worldPosition, bool updateWorldTransformationMatrix = true) override;
	virtual void SetWorldRotation(const Quaternion& worldRotation, bool updateWorldTransformationMatrix = true) override;

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

	void ClearForces();
	void ClearGravity();

	btRigidBody* GetBulletRigidBody() const
	{
		return bulletRigidBody_;
	}

protected:
	btRigidBody* bulletRigidBody_{ nullptr };

private:
	RigidBodyInitializationData* initializationData_{ nullptr };

	float mass_{ 0.f };
};

#endif