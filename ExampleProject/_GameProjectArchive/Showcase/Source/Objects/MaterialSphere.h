#pragma once

#include "Goknar/Core.h"
#include "Goknar/ObjectBase.h"

class StaticMeshComponent;

class GOKNAR_API MaterialSphere : public ObjectBase
{
public:
	MaterialSphere();

	StaticMeshComponent* GetSphereMeshComponent()
	{
		return sphereMeshComponent_;
	}

protected:

private:
	StaticMeshComponent* sphereMeshComponent_;
};