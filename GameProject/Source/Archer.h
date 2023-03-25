#pragma once

#include "Goknar/ObjectBase.h"

class Camera;
class SkeletalMesh;
class SkeletalMeshComponent;

class ArcherGameController;
class ArcherMovementComponent;

class Archer : public ObjectBase
{
public:
	Archer();
	virtual void BeginGame() override;
    virtual void Tick(float deltaTime) override;

	SkeletalMesh* GetSkeletalMesh() const
	{
		return skeletalMesh_;
	}

	SkeletalMeshComponent* GetSkeletalMeshComponent() const
	{
		return skeletalMeshComponent_;
	}

	ArcherMovementComponent* GetMovementComponent() const
	{
		return movementComponent_;
	}

	ArcherGameController* GetController() const
	{
		return controller_;
	}

	Camera* GetThirdPersonCamera() const
	{
		return thirdPersonCamera_;
	}

private:
	SkeletalMesh* skeletalMesh_;
	SkeletalMeshComponent* skeletalMeshComponent_;

	Camera* thirdPersonCamera_;

	ArcherGameController* controller_;
	ArcherMovementComponent* movementComponent_;
};

