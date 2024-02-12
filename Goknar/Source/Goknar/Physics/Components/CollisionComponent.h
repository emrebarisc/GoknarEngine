#ifndef __COLLISIONCOMPONENT_H__
#define __COLLISIONCOMPONENT_H__

#include "Components/Component.h"
#include "Delegates/Delegate.h"
#include "Physics/PhysicsTypes.h"

class btCollisionShape;
class btCollisionObject;

class PhysicsObject;

using OverlapBeginAlias = void(PhysicsObject*, class CollisionComponent*, const Vector3&, const Vector3&);
using OverlapContinueAlias = void(PhysicsObject*, class CollisionComponent*, const Vector3&, const Vector3&);
using OverlapEndAlias = void(PhysicsObject*, class CollisionComponent*);

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