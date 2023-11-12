#pragma once

#include "Goknar/ObjectBase.h"

class SkeletalMesh;
class SkeletalMeshComponent;

class Dancer : public ObjectBase
{
public:
	Dancer();
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

private:
	SkeletalMesh* skeletalMesh_;
	SkeletalMeshComponent* skeletalMeshComponent_;
};

