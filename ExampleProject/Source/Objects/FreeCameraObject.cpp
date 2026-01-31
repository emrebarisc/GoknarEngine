#include "FreeCameraObject.h"

#include "Goknar/Engine.h"
#include "Goknar/Camera.h"
#include "Goknar/Managers/WindowManager.h"

#include "Components/CameraComponent.h"
#include "Controllers/FreeCameraController.h"

FreeCameraObject::FreeCameraObject() : ObjectBase()
{
	cameraComponent_ = AddSubComponent<CameraComponent>();
	cameraComponent_->SetCameraFollowsComponentRotation(true);
	SetRootComponent(cameraComponent_);

	freeCameraController_ = new FreeCameraController(this);
}

void FreeCameraObject::BeginGame()
{
	WindowManager* windowManager = engine->GetWindowManager();
	Camera* camera = cameraComponent_->GetCamera();
	camera->SetImageWidth(windowManager->GetWindowSize().x);
	camera->SetImageHeight(windowManager->GetWindowSize().y);

	SetWorldPosition(Vector3{ -15.f, -16.f, 12.5f });
	SetWorldRotation(Quaternion::FromEulerDegrees({ 5.f, 20.f, 25.f }));
}

void FreeCameraObject::Tick(float deltaTime)
{

}
