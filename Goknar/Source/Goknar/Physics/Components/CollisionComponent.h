#ifndef __COLLISIONCOMPONENT_H__
#define __COLLISIONCOMPONENT_H__

#include "Components/Component.h"
#include "Delegates/Delegate.h"
#include "Physics/PhysicsTypes.h"

class btCollisionShape;
class btCollisionObject;

class ObjectBase;

using OverlapCollisionAlias = void(ObjectBase*, class CollisionComponent*, const Vector3&, const Vector3&);

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

	btCollisionObject* GetBulletCollisionObject() const
	{
		return bulletCollisionObject_;
	}

	inline bool GetIsOverlapping() const
	{
		return isOverlapping_;
	}

	void SetIsOverlapping(bool isOverlapping);

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

	Delegate<OverlapCollisionAlias> OnOverlapBegin;
	Delegate<OverlapCollisionAlias> OnOverlapContinue;
	Delegate<OverlapCollisionAlias> OnOverlapEnd;

protected:
	btCollisionShape* bulletCollisionShape_{ nullptr };
	btCollisionObject* bulletCollisionObject_{ nullptr };
private:
	CollisionGroup collisionGroup_{ CollisionGroup::WorldDynamic };
	CollisionMask collisionMask_{ CollisionMask::BlockAll };

	bool isOverlapping_{ false };
};

#endif