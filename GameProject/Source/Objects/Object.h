#pragma once

#include "Goknar/Physics/PhysicsObject.h"

class StaticMeshComponent;

class Object : public PhysicsObject
{
public:
	Object();

private:
	StaticMeshComponent* staticMeshComponent_;
};