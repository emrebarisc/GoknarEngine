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

	float GetMass() const
	{
		return mass_;
	}

	void SetMass(float mass)
	{
		mass_ = mass;
	}

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

	btRigidBody* GetBulletRigidBody() const
	{
		return bulletRigidBody_;
	}

protected:
	btRigidBody* bulletRigidBody_;

private:
	btVector3 localInertia_;
	float mass_;
};

#endif