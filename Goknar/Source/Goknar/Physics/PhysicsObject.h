#ifndef __PHYSICSOBJECT_H__
#define __PHYSICSOBJECT_H__

#include "Core.h"

#include "Goknar/ObjectBase.h"
#include "Goknar/Physics/PhysicsTypes.h"

class btCollisionObject;

struct GOKNAR_API PhysicsObjectInitializationData
{
	float ccdMotionThreshold{ 0.f };
	float ccdSweptSphereRadius{ 0.f };

	float linearSleepingThreshold{ -1.f };
	float angularSleepingThreshold{ -1.f };
};

class GOKNAR_API PhysicsObject : public ObjectBase
{
public:
	PhysicsObject();
	~PhysicsObject();

	virtual void PreInit();
	virtual void Init();
	virtual void PostInit();

	virtual void PhysicsTick(float deltaTime) {}

	void SetupPhysicsObjectInitializationData();

	void SetCcdMotionThreshold(float ccdMotionThreshold);
	void SetCcdSweptSphereRadius(float ccdSweptSphereRadius);

	void SetLinearSleepingThreshold(float linearSleepingThreshold);
	void SetAngularSleepingThreshold(float angularSleepingThreshold);

	btCollisionObject* GetBulletCollisionObject() const
	{
		return bulletCollisionObject_;
	}

	CollisionGroup GetCollisionGroup() const
	{
		return collisionGroup_;
	}

	void SetCollisionGroup(CollisionGroup collisionGroup)
	{
		collisionGroup_ = collisionGroup;
	}

	CollisionMask GetCollisionMask() const
	{
		return collisionMask_;
	}

	void SetCollisionMask(CollisionMask collisionMask)
	{
		collisionMask_ = collisionMask;
	}

protected:
	btCollisionObject* bulletCollisionObject_{ nullptr };

	PhysicsObjectInitializationData* physicsObjectInitializationData_{ nullptr };

    CollisionGroup collisionGroup_{ CollisionGroup::WorldDynamicBlock };
    CollisionMask collisionMask_{ CollisionMask::BlockAll };
private:
};

#endif