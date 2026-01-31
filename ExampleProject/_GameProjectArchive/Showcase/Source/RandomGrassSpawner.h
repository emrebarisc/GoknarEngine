#pragma once

#include "Goknar/ObjectBase.h"

class RandomGrassSpawner : public ObjectBase
{
public:
	RandomGrassSpawner();

protected:
	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;

private:
};