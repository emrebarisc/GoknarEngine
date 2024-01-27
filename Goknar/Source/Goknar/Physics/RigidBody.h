#ifndef __RIGIDBODY_H__
#define __RIGIDBODY_H__

#include "Core.h"

#include "Goknar/ObjectBase.h"

#include "LinearMath/btVector3.h"

class btRigidBody;

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

	Vector3 GetLocalInertia() const
	{
		return Vector3{ localInertia_.x(), localInertia_.y(), localInertia_.z() };
	}

	void SetLocalInertia(const Vector3& localInertia)
	{
		localInertia_.setX(localInertia.x);
		localInertia_.setY(localInertia.y);
		localInertia_.setZ(localInertia.z);
	}

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
	btVector3 initialVelocity_{ btVector3(0.f, 0.f, 0.f) };
	btVector3 initialAngularVelocity_{ btVector3(0.f, 0.f, 0.f) };
	
	btVector3 initialForce_{ btVector3(0.f, 0.f, 0.f) };
	btVector3 initialForcePosition_{ btVector3(0.f, 0.f, 0.f) };
	btVector3 initialCentralImpulse_{ btVector3(0.f, 0.f, 0.f) };
	btVector3 initialTorqueImpulse_{ btVector3(0.f, 0.f, 0.f) };
	btVector3 initialPushImpulse_{ btVector3(0.f, 0.f, 0.f) };
	btVector3 initialPushImpulsePosition_{ btVector3(0.f, 0.f, 0.f) };

	btVector3 localInertia_{ btVector3(0.f, 0.f, 0.f) };
	float mass_{ 0.f };
};

#endif