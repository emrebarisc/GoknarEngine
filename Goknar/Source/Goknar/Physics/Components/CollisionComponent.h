#ifndef __COLLISIONCOMPONENT_H__
#define __COLLISIONCOMPONENT_H__

#include "Components/Component.h"

class btCollisionShape;

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

protected:
	btCollisionShape* bulletCollisionShape_{ nullptr };
private:
};

#endif