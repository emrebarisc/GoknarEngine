#ifndef __SPHERECOLLISIONCOMPONENT_H__
#define __SPHERECOLLISIONCOMPONENT_H__

#include "CollisionComponent.h"

class GOKNAR_API SphereCollisionComponent : public CollisionComponent
{
public:
	SphereCollisionComponent(Component* parent);
	SphereCollisionComponent(ObjectBase* parentObjectBase);
	~SphereCollisionComponent();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;
	
	virtual void BeginGame() override;
	virtual void TickComponent(float deltaTime) override;

	float GetRadius() const
	{
		return radius_;
	}

	void SetRadius(float radius);

protected:
private:
	float radius_{ 1.f };
};

#endif