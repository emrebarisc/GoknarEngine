#ifndef __CAPSULECOLLISIONCOMPONENT_H__
#define __CAPSULECOLLISIONCOMPONENT_H__

#include "CollisionComponent.h"

class GOKNAR_API CapsuleCollisionComponent : public CollisionComponent
{
public:
	CapsuleCollisionComponent(Component* parent);
	CapsuleCollisionComponent(ObjectBase* parentObjectBase);
	~CapsuleCollisionComponent();

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

	float GetHeight() const
	{
		return height_;
	}

	void SetHeight(float height);

protected:
private:
	float radius_{ 1.f };
	float height_{ 1.f };
};

#endif