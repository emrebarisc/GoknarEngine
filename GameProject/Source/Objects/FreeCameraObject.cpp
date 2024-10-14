#include "FreeCameraObject.h"

#include "Goknar/Engine.h"
#include "Goknar/Camera.h"
#include "Goknar/Managers/WindowManager.h"

#include "Components/CameraComponent.h"
#include "Controllers/FreeCameraController.h"

#include "Arrow.h"

FreeCameraObject::FreeCameraObject() : ObjectBase()
{
	SetIsTickable(true);
	SetIsTickEnabled(false);

	cameraComponent_ = AddSubComponent<CameraComponent>();
	cameraComponent_->SetCameraFollowsComponentRotation(true);
	cameraComponent_->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3{ 0.f, 0.f, 45.f }));
	SetRootComponent(cameraComponent_);

	freeCameraController_ = new FreeCameraController(this);
	freeCameraController_->SetName("FreeCameraController");
}

void FreeCameraObject::SetFollowObject(ObjectBase* followObject)
{
	SetWorldRotation(Vector3{ 1.f, 1.f, -1.f }.GetRotationNormalized());

	if (dynamic_cast<Arrow*>(followObject))
	{
		SetIsTickEnabled(false);
		SetWorldPosition(followObject->GetWorldPosition() - Vector3{ 1.f, 1.f, -1.f } * 4.f);
		SetParent(followObject);
	}
	else
	{
		SetIsTickEnabled(true);
	}

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
