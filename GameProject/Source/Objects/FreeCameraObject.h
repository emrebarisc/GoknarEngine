#pragma once

#include "Goknar/Core.h"
#include "Goknar/ObjectBase.h"

class FreeCameraController;

class GOKNAR_API FreeCameraObject : public ObjectBase
{
public:
	FreeCameraObject();

	FreeCameraController* GetFreeCameraController() const 
	{
		return freeCameraController_;
	}

protected:
	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;

	FreeCameraController* freeCameraController_;
private:
};