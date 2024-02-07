#ifndef __COLLISIONCOMPONENT_H__
#define __COLLISIONCOMPONENT_H__

#include "Components/Component.h"
#include "Delegates/Delegate.h"

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

	Delegate<OverlapCollisionAlias> OnOverlapBegin;
	Delegate<OverlapCollisionAlias> OnOverlapContinue;
	Delegate<OverlapCollisionAlias> OnOverlapEnd;

protected:
	btCollisionShape* bulletCollisionShape_{ nullptr };
	btCollisionObject* bulletCollisionObject_{ nullptr };
private:

	bool isOverlapping_{ false };
};

#endif