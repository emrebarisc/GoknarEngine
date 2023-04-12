#pragma once

#include "StaticMeshObject.h"

class ProjectileMovementComponent;

class GOKNAR_API Arrow : public StaticMeshObject
{
public:
	Arrow();

	virtual void BeginGame() override;

protected:

private:
	ProjectileMovementComponent* movementComponent_;
};