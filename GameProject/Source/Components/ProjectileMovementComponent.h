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

	void Shoot();

protected:
	virtual void BeginGame() override;
	virtual void TickComponent(float deltaTime) override;

private:
	float movementSpeed_{ 4.f };
};


#endif