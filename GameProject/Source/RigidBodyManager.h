#pragma once

#include "Goknar/ObjectBase.h"

class AeroControl;

class RigidBodyManager : public ObjectBase
{
public:
	RigidBodyManager();

	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;

	AeroControl* aeroControlForceGenerator;

	Vector3 anchoredSpringAnchor_;
	Vector3 anchoredSpringWithRelativePositionAnchor_;
	Vector3 anchoredBungeeAnchor_;
	Vector3 fakeSpringAnchor_;

	Vector3 windSpeed_;
};