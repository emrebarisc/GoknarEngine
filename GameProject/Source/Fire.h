#pragma once

#include "Goknar/ObjectBase.h"

class PointLight;

class Fire : public ObjectBase
{
public:
	Fire();

	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;

private:
	PointLight* fireLight_;

	float fireIntensity_{ 10.f };
};