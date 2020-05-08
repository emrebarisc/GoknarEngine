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
#include "Goknar/Managers/WindowManager.h"

#include "Goknar/Log.h"


CameraController::CameraController() : 
	ObjectBase(),
	isRotatingTheCamera_(false),
	isMovingCameraIn2D_(false)
{
	SetTickable(false);

	activeCamera_ = engine->GetCameraManager()->GetActiveCamera();
	movementSpeed_ = 10.f;
	previousCursorPositionForRotating_ = Vector2(0.f, 0.f);
	previousCursorPositionFor2DMovement_ = Vector2(0.f, 0.f);

	InputManager* inputManager = engine->GetInputManager();

	GOKNAR_ASSERT(inputManager, "Input manager is NULL.");
	if (inputManager)
	{
		inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_PRESS, std::bind(&CameraController::OnMouseRightClickPressed, this));
		inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_RELEASE, std::bind(&CameraController::OnMouseRightClickReleased, this));

		inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_PRESS, std::bind(&CameraController::OnMouseMiddleClickPressed, this));
		inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_RELEASE, std::bind(&CameraController::OnMouseMiddleClickReleased, this));

		inputManager->AddScrollDelegate(std::bind(&CameraController::ScrollListener, this, std::placeholders::_1, std::placeholders::_2));

		inputManager->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_REPEAT, std::bind(&CameraController::MoveLeftListener, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_REPEAT, std::bind(&CameraController::MoveLeftListener, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_REPEAT, std::bind(&CameraController::MoveRightListener, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_REPEAT, std::bind(&CameraController::MoveForwardListener, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_REPEAT, std::bind(&CameraController::MoveBackwardListener, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::LEFT_CONTROL, INPUT_ACTION::G_REPEAT, std::bind(&CameraController::MoveDownListener, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_REPEAT, std::bind(&CameraController::MoveUpListener, this));

		inputManager->AddCursorDelegate(std::bind(&CameraController::CursorMovement, this, std::placeholders::_1, std::placeholders::_2));
	}
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

void CameraController::CursorMovement(double x, double y)
{
	Vector2 currentCursorPosition(x, y);

	if (isRotatingTheCamera_)
	{
		Vector2 cursorMovementVector = (currentCursorPosition - previousCursorPositionForRotating_) / 250.f;
		Yaw(cursorMovementVector.x);
		Pitch(cursorMovementVector.y);

		previousCursorPositionForRotating_ = currentCursorPosition;
	}

	if (isMovingCameraIn2D_)
	{
		Vector2 cursorMovementVector = currentCursorPosition - previousCursorPositionFor2DMovement_;

		MoveRight(cursorMovementVector.x);
		MoveUp(-cursorMovementVector.y);

		previousCursorPositionFor2DMovement_ = currentCursorPosition;
	}
}

void CameraController::ScrollListener(double x, double y)
{
	MoveForward(y * movementSpeed_);
}

void CameraController::Yaw(float value)
{
	activeCamera_->RotateAbout(Vector3::UpVector, value);
}

void CameraController::Pitch(float value)
{
	activeCamera_->Pitch(value);
}

void CameraController::OnMouseRightClickPressed()
{
	double x, y;
	InputManager::GetCursorPosition(engine->GetWindowManager()->GetWindow(), x, y);
	previousCursorPositionForRotating_ = Vector2(x, y);
	isRotatingTheCamera_ = true;
}

void CameraController::OnMouseMiddleClickPressed()
{
	double x, y;
	InputManager::GetCursorPosition(engine->GetWindowManager()->GetWindow(), x, y);
	previousCursorPositionFor2DMovement_ = Vector2(x, y);
	isMovingCameraIn2D_ = true;
}

void CameraController::MoveForward(float multiplier/* = 1.f*/)
{
	activeCamera_->SetPosition(activeCamera_->GetPosition() + activeCamera_->GetForwardVector() * engine->GetDeltaTime() * multiplier);
}

void CameraController::MoveRight(float multiplier/* = 1.f*/)
{
	activeCamera_->SetPosition(activeCamera_->GetPosition() + activeCamera_->GetRightVector() * engine->GetDeltaTime() * multiplier);
}

void CameraController::MoveUp(float multiplier/* = 1.f*/)
{
	activeCamera_->SetPosition(activeCamera_->GetPosition() + Vector3::UpVector * engine->GetDeltaTime() * multiplier);
}