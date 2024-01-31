#ifndef __BOXCOLLISIONCOMPONENT_H__
#define __BOXCOLLISIONCOMPONENT_H__

#include "CollisionComponent.h"

class GOKNAR_API BoxCollisionComponent : public CollisionComponent
{
public:
	BoxCollisionComponent(Component* parent);
	BoxCollisionComponent(ObjectBase* parentObjectBase);
	~BoxCollisionComponent();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;
	
	virtual void BeginGame() override;
	virtual void TickComponent(float deltaTime) override;

	const Vector3& GetHalfSize() const
	{
		return halfSize_;
	}

	void SetHalfSize(const Vector3& halfSize);

protected:
	Vector3 halfSize_{ Vector3{1.f} };
private:
};

#endif