#pragma once

#include "StaticMeshObject.h"

class ProjectileMovementComponent;

class GOKNAR_API Arrow : public StaticMeshObject
{
public:
	Arrow();

	virtual void BeginGame() override;

	ProjectileMovementComponent* GetMovementComponent() const
	{
		return movementComponent_;
	}

protected:

private:
	ProjectileMovementComponent* movementComponent_;
};