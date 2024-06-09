#pragma once

#include "Goknar/ObjectBase.h"

class SpotLight;

class LightController : public ObjectBase
{
public:
	LightController();

	void BeginGame();
	void Tick(float deltaTime);

private:
	SpotLight* spotLight1_;
	SpotLight* spotLight2_;
	SpotLight* spotLight3_;
	SpotLight* spotLight4_;
	SpotLight* spotLight5_;

	Vector3 centerPosition_{ Vector3{-40.f, 40.f, 0.f} };

	float angle_;
	float radius_;
	float rotationSpeed_;
};