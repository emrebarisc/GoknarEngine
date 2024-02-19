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
	int collisionFlag{ -1 };
};

class GOKNAR_API PhysicsObject : public ObjectBase
{
public:
	PhysicsObject();
	~PhysicsObject();

	virtual void PreInit();
	virtual void Init();
	virtual void PostInit();

	virtual void Destroy() override;

	virtual void SetIsActive(bool isActive) override;

	virtual void PhysicsTick(float deltaTime) {}

	void SetupPhysicsObjectInitializationData();

	void SetCcdMotionThreshold(float ccdMotionThreshold);
	void SetCcdSweptSphereRadius(float ccdSweptSphereRadius);

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

	void SetCollisionFlag(CollisionFlag collisionFlag);
	CollisionFlag GetCollisionFlag() const;
	
	const std::string& GetTag() const
	{
		return tag_;
	}

	void SetTag(const std::string& tag)
	{
		tag_ = tag;
	}

protected:
	btCollisionObject* bulletCollisionObject_{ nullptr };

	PhysicsObjectInitializationData* physicsObjectInitializationData_{ nullptr };

    CollisionGroup collisionGroup_{ CollisionGroup::WorldDynamicBlock };
    CollisionMask collisionMask_{ CollisionMask::BlockAndOverlapAll };

	std::string tag_{ "None" };
private:
};

#endif