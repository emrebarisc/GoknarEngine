#pragma once

#include "Goknar/ObjectBase.h"

class Camera;
class SkeletalMesh;
class SkeletalMeshComponent;

class Archer : public ObjectBase
{
public:
	Archer();
	virtual void BeginGame() override;
    virtual void Tick(float deltaTime) override;

	SkeletalMesh* GetSkeletalMesh() const
	{
		return skeletalMesh;
	}

	SkeletalMeshComponent* GetSkeletalMeshComponent() const
	{
		return skeletalMeshComponent;
	}

private:
	SkeletalMesh* skeletalMesh;
	SkeletalMeshComponent* skeletalMeshComponent;

	Camera* followCamera;
};

