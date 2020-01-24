#include "CameraController.h"

#include "Goknar/Scene.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Model/Mesh.h"
#include "Goknar/Components/MeshComponent.h"
#include "Goknar/Managers/InputManager.h"

#include "Goknar/Log.h"


CameraController::CameraController() : ObjectBase()
{
	SetTickable(false);

	activeCamera_ = engine->GetCameraManager()->GetActiveCamera();
	movementSpeed_ = 10.f;

	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_REPEAT, std::bind(&CameraController::MoveLeft, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_REPEAT, std::bind(&CameraController::MoveRight, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_REPEAT, std::bind(&CameraController::MoveForward, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_REPEAT, std::bind(&CameraController::MoveBackward, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::LEFT_CONTROL, INPUT_ACTION::G_REPEAT, std::bind(&CameraController::MoveDown, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_REPEAT, std::bind(&CameraController::MoveUp, this));
}

CameraController::~CameraController()
{
}

void CameraController::BeginGame()
{
	GOKNAR_CORE_INFO("CameraController Begin Game");
}

void CameraController::Tick(float deltaTime)
{
}

void CameraController::PositiveYaw()
{
}

void CameraController::NegativeYaw()
{
}

void CameraController::PositivePitch()
{
}

void CameraController::NegativePitch()
{
}

void CameraController::MoveForward()
{
	activeCamera_->SetPosition(activeCamera_->GetPosition() + activeCamera_->GetForwardVector() * engine->GetDeltaTime() * movementSpeed_);
}

void CameraController::MoveBackward()
{
	activeCamera_->SetPosition(activeCamera_->GetPosition() - activeCamera_->GetForwardVector() * engine->GetDeltaTime() * movementSpeed_);
}

void CameraController::MoveRight()
{
	activeCamera_->SetPosition(activeCamera_->GetPosition() + activeCamera_->GetRightVector() * engine->GetDeltaTime() * movementSpeed_);
}

void CameraController::MoveLeft()
{
	activeCamera_->SetPosition(activeCamera_->GetPosition() - activeCamera_->GetRightVector() * engine->GetDeltaTime() * movementSpeed_);
}

void CameraController::MoveUp()
{
	activeCamera_->SetPosition(activeCamera_->GetPosition() + activeCamera_->GetUpVector() * engine->GetDeltaTime() * movementSpeed_);
}

void CameraController::MoveDown()
{
	activeCamera_->SetPosition(activeCamera_->GetPosition() - activeCamera_->GetUpVector() * engine->GetDeltaTime() * movementSpeed_);
}
