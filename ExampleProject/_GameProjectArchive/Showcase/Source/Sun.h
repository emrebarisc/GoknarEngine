#pragma once

#include "Goknar/ObjectBase.h"

class DirectionalLight;

class Sun : public ObjectBase
{
public:
	Sun();
	~Sun();

	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;

private:
	DirectionalLight* sunLight_;

	Vector3 sunLightDirection_{ Vector3(0.57735f, 0.57735f, -0.57735f) };
	Vector3 sunRotationAxis_{ Vector3::ForwardVector };
	float sunLightRotationSpeed_{ 1.f };
};