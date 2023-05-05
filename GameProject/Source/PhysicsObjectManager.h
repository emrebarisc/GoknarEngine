#pragma once

#include "Goknar/ObjectBase.h"

class PhysicsObjectManager : public ObjectBase
{
public:
	PhysicsObjectManager();

private:
	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;
};