#pragma once

#include "Goknar/Physics/RigidBody.h"

class StaticMeshComponent;

class RigidBodyObject : public RigidBody
{
public:
	RigidBodyObject();

private:
	StaticMeshComponent* staticMeshComponent_;
};