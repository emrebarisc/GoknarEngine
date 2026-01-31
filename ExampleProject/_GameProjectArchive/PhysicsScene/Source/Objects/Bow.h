#pragma once

#include "StaticMeshObject.h"

class SkeletalMeshComponent;

class GOKNAR_API Bow : public ObjectBase//StaticMeshObject
{
public:
	Bow();

	SkeletalMeshComponent* GetSkeletalMesh()
	{
		return skeletalMeshComponent_;
	}

protected:

private:
	SkeletalMeshComponent* skeletalMeshComponent_;
};