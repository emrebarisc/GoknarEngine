#pragma once

#include "Goknar/ObjectBase.h"

class SpotLight;

class LightController : public ObjectBase
{
public:
	LightController();

	void BeginPlay();
	void Tick(float deltaTime);

private:
	SpotLight* spotLight1;
	SpotLight* spotLight2;
	SpotLight* spotLight3;
	SpotLight* spotLight4;
	SpotLight* spotLight5;

	float angle;
	float radius;
	float rotationSpeed;
};