#pragma once

#include "Goknar/Physics/RigidBody.h"

class StaticMeshComponent;

class Ball : public RigidBody
{
public:
	Ball();

private:
	StaticMeshComponent* staticMeshComponent_;
};