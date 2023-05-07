#pragma once

#include "Goknar/ObjectBase.h"

class RigidBodyManager : public ObjectBase
{
public:
	RigidBodyManager();

private:
	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;

	Vector3 anchoredSpringAnchor_;
	Vector3 anchoredBungeeAnchor_;
	Vector3 fakeSpringAnchor_;
};