#pragma once

#include "Goknar/ObjectBase.h"

class SkeletalMesh;
class SkeletalMeshComponent;

class Mutant : public ObjectBase
{
public:
	Mutant();
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
};

