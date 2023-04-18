#ifndef __PROJECTILEMOVEMENTCOMPONENT_H__
#define __PROJECTILEMOVEMENTCOMPONENT_H__

#include "Goknar/Components/Component.h"

#include "InterpolatedValue.h"

class Camera;
class SkeletalMeshComponent;
class SkeletalMeshInstance;

class ProjectileMovementComponent : public Component
{
public:
	ProjectileMovementComponent(Component* parent);

	virtual void BeginGame() override;
	virtual void TickComponent(float deltaTime) override;

	void SetIsActive(bool isActive)
	{
		isActive_ = isActive;
	}

	bool GetIsActive() const
	{
		return isActive_;
	}

protected:

private:
	Vector3 velocity_{ Vector3::ZeroVector };
	float movementSpeed_{ 40.f };
	bool isActive_{ false };
};


#endif