#pragma once

#include "Goknar/Core.h"
#include "Goknar/ObjectBase.h"

class CameraComponent;
class FreeCameraController;

class GOKNAR_API FreeCameraObject : public ObjectBase
{
public:
	FreeCameraObject();

	FreeCameraController* GetFreeCameraController() const 
	{
		return freeCameraController_;
	}

	CameraComponent* GetCameraComponent() const
	{
		return cameraComponent_;
	}

	void SetFollowObject(ObjectBase* followObject);

protected:
	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;

	FreeCameraController* freeCameraController_;
	CameraComponent* cameraComponent_;
private:

	ObjectBase* followObject_{ nullptr };
};