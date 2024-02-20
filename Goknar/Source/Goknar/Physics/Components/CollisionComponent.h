#ifndef __COLLISIONCOMPONENT_H__
#define __COLLISIONCOMPONENT_H__

#include "Components/Component.h"
#include "Delegates/Delegate.h"
#include "Physics/PhysicsTypes.h"

class btCollisionShape;
class btCollisionObject;

class PhysicsObject;

using OverlapBeginAlias = void(PhysicsObject* /*otherObject*/, class CollisionComponent* /*otherComponent*/, const Vector3& /*hitPosition*/, const Vector3& /*hitNormal*/);
using OverlapContinueAlias = void(PhysicsObject* /*otherObject*/, class CollisionComponent* /*otherComponent*/, const Vector3& /*hitPosition*/, const Vector3& /*hitNormal*/);
using OverlapEndAlias = void(PhysicsObject* /*otherObject*/, class CollisionComponent* /*otherComponent*/);

class GOKNAR_API CollisionComponent : public Component
{
public:
	CollisionComponent(Component* parent);
	CollisionComponent(ObjectBase* parentObjectBase);
	~CollisionComponent();

	virtual void UpdateComponentToWorldTransformationMatrix() override;

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;
	
	virtual void BeginGame() override;
	virtual void TickComponent(float deltaTime) override;

	virtual void UpdateTransformation();

	btCollisionShape* GetBulletCollisionShape() const
	{
		return bulletCollisionShape_;
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

	void OverlapBegin(PhysicsObject* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal)
	{
		// TODO: Find a better way for active controlling.
		// Disabling bullet overlapping on bullet side is the best way but
		// Not working with btGhostObject(it works fine with btRigidBody)
		if(isActive_ && !OnOverlapBegin.isNull())
		{
			OnOverlapBegin(otherObject, otherComponent, hitPosition, hitNormal);
		}
	}

	void OverlapContinue(PhysicsObject* otherObject, CollisionComponent* otherComponent, const Vector3& hitPosition, const Vector3& hitNormal)
	{
		// TODO: Find a better way for active controlling.
		// Disabling bullet overlapping on bullet side is the best way but
		// Not working with btGhostObject(it works fine with btRigidBody)
		if(isActive_ && !OnOverlapContinue.isNull())
		{
			OnOverlapContinue(otherObject, otherComponent, hitPosition, hitNormal);
		}
	}

	void OverlapEnd(PhysicsObject* otherObject, CollisionComponent* otherComponent)
	{
		// TODO: Find a better way for active controlling.
		// Disabling bullet overlapping on bullet side is the best way but
		// Not working with btGhostObject(it works fine with btRigidBody)
		if(isActive_ && !OnOverlapEnd.isNull())
		{
			OnOverlapEnd(otherObject, otherComponent);
		}
	}	

	Delegate<OverlapBeginAlias> OnOverlapBegin;
	Delegate<OverlapContinueAlias> OnOverlapContinue;
	Delegate<OverlapEndAlias> OnOverlapEnd;

protected:
	btCollisionShape* bulletCollisionShape_{ nullptr };

private:
	CollisionGroup collisionGroup_{ CollisionGroup::All };
	CollisionMask collisionMask_{ CollisionMask::BlockAndOverlapAll };
};

#endif