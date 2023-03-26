#ifndef __ARCHERMOVEMENTCOMPONENT_H__
#define __ARCHERMOVEMENTCOMPONENT_H__

#include "Goknar/Components/Component.h"

class Archer;
class Camera;
class SkeletalMeshComponent;

class ArcherMovementComponent : public Component
{
public:
	ArcherMovementComponent(Component* parent);

	virtual void BeginGame() override;
	virtual void TickComponent(float deltaTime) override;

	void AddMovementVector(const Vector3& movement)
	{
		movementVector_ += movement;
	}

	void SetOwnerArcher(Archer* ownerArcher)
	{
		ownerArcher_ = ownerArcher;
	}

protected:

private:
	Vector3 movementVector_;

	Archer* ownerArcher_;
	Camera* thirdPersonCamera_;
	SkeletalMeshComponent* archerSkeletalMeshComponent_;

	float movementSpeed_;
};


#endif