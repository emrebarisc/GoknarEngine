#include "FreeCameraObject.h"

#include "Goknar/Engine.h"
#include "Goknar/Camera.h"
#include "Goknar/Managers/WindowManager.h"

#include "Components/CameraComponent.h"
#include "Controllers/FreeCameraController.h"

FreeCameraObject::FreeCameraObject() : ObjectBase()
{
	SetIsTickable(true);

	cameraComponent_ = AddSubComponent<CameraComponent>();
	cameraComponent_->SetCameraFollowsComponentRotation(true);
	SetRootComponent(cameraComponent_);

	freeCameraController_ = new FreeCameraController(this);
	freeCameraController_->SetName("FreeCameraController");
}

void FreeCameraObject::SetFollowObject(ObjectBase* followObject)
{
	SetWorldRotation(Vector3{ 1.f, 1.f, -1.f }.GetRotationNormalized());
	followObject_ = followObject;
}

void FreeCameraObject::BeginGame()
{
	WindowManager* windowManager = engine->GetWindowManager();
	Camera* camera = cameraComponent_->GetCamera();
	camera->SetImageWidth(windowManager->GetWindowSize().x);
	camera->SetImageHeight(windowManager->GetWindowSize().y);

	SetWorldPosition(Vector3{ 15.f, 16.f, 12.5f });
	SetWorldRotation(Quaternion::FromEulerDegrees({ 5.f, 20.f, 205.f }));
}

void FreeCameraObject::Tick(float deltaTime)
{
	if (followObject_)
	{
		SetWorldPosition(followObject_->GetWorldPosition() - GetForwardVector() * 4.f);
	}
}
