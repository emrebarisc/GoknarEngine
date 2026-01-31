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

	Vector3 GetSpringPosition(float deltaTime);

	FreeCameraController* freeCameraController_;
	CameraComponent* cameraComponent_;
private:

	ObjectBase* followObject_{ nullptr };
	Vector3 currentVelocity_{ Vector3::ZeroVector };
};