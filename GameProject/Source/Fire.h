#pragma once

#include "Goknar/ObjectBase.h"

class PointLight;

class Fire : public ObjectBase
{
public:
	Fire();
	~Fire();

	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;

	virtual void SetWorldPosition(const Vector3& position, bool updateWorldTransformationMatrix = true) override;

private:
	PointLight* fireLight_;

	float fireIntensity_{ 100.f };
};