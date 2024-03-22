#ifndef __MULTIPLECOLLISIONCOMPONENT_H__
#define __MULTIPLECOLLISIONCOMPONENT_H__

#include <vector>

#include "CollisionComponent.h"

class btCompoundShape;

class GOKNAR_API MultipleCollisionComponent : public CollisionComponent
{
public:
	MultipleCollisionComponent(Component* parent);
	MultipleCollisionComponent(ObjectBase* parentObjectBase);
	~MultipleCollisionComponent();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;
	
	virtual void BeginGame() override;
	virtual void TickComponent(float deltaTime) override;

	virtual void AddCollisionComponent(CollisionComponent* collisionComponent);

	btCompoundShape* GetBulletCompoundShape() const
	{
		return bulletCompoundShape_;
	}

protected:
private:
	void AddCollisionComponentInner(CollisionComponent* collisionComponent);

	btCompoundShape* bulletCompoundShape_{ nullptr };

	std::vector<CollisionComponent*>* subCollisionComponents_{ new std::vector<CollisionComponent*>() };
};

#endif