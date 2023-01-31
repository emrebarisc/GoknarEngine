#include "pch.h"

#include "EditorCameraController.h"

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


EditorCameraController::EditorCameraController() : 
	ObjectBase(),
	isRotatingTheCamera_(false),
	isMovingCameraIn2D_(false),
	activeCamera_(nullptr)
{
	SetTickable(false);

	movementSpeed_ = 50.f;
	previousCursorPositionForRotating_ = Vector2(0.f, 0.f);
	previousCursorPositionFor2DMovement_ = Vector2(0.f, 0.f);
}

EditorCameraController::~EditorCameraController()
{
}

void EditorCameraController::BeginGame()
{
	GOKNAR_CORE_INFO("CameraController Begin Game");

	InputManager* inputManager = engine->GetInputManager();

	GOKNAR_ASSERT(inputManager, "Input manager is NULL.");
	if (inputManager)
	{
		inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_PRESS, std::bind(&EditorCameraController::OnMouseRightClickPressed, this));
		inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_RELEASE, std::bind(&EditorCameraController::OnMouseRightClickReleased, this));

		inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_PRESS, std::bind(&EditorCameraController::OnMouseMiddleClickPressed, this));
		inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_RELEASE, std::bind(&EditorCameraController::OnMouseMiddleClickReleased, this));

		inputManager->AddScrollDelegate(std::bind(&EditorCameraController::ScrollListener, this, std::placeholders::_1, std::placeholders::_2));

		inputManager->AddKeyboardInputDelegate(KEY_MAP::LEFT, INPUT_ACTION::G_REPEAT, std::bind(&EditorCameraController::MoveLeftListener, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::RIGHT, INPUT_ACTION::G_REPEAT, std::bind(&EditorCameraController::MoveRightListener, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::UP, INPUT_ACTION::G_REPEAT, std::bind(&EditorCameraController::MoveForwardListener, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::DOWN, INPUT_ACTION::G_REPEAT, std::bind(&EditorCameraController::MoveBackwardListener, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::LEFT_CONTROL, INPUT_ACTION::G_REPEAT, std::bind(&EditorCameraController::MoveDownListener, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_REPEAT, std::bind(&EditorCameraController::MoveUpListener, this));

		inputManager->AddCursorDelegate(std::bind(&EditorCameraController::CursorMovement, this, std::placeholders::_1, std::placeholders::_2));
	}
}

void EditorCameraController::Tick(float deltaTime)
{
}

void EditorCameraController::CursorMovement(double x, double y)
{
	Vector2 currentCursorPosition(x, y);

	if (isRotatingTheCamera_)
	{
		Vector2 cursorMovementVector = (previousCursorPositionForRotating_ - currentCursorPosition) / 250.f;
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

void EditorCameraController::ScrollListener(double x, double y)
{
	MoveForward(y * movementSpeed_);
}

void EditorCameraController::Yaw(float value)
{
	activeCamera_ = engine->GetCameraManager()->GetActiveCamera();
	activeCamera_->RotateAbout(Vector3::UpVector, value);
}

void EditorCameraController::Pitch(float value)
{
	activeCamera_ = engine->GetCameraManager()->GetActiveCamera();
	activeCamera_->Pitch(value);
}

void EditorCameraController::OnMouseRightClickPressed()
{
	double x, y;
	InputManager::GetCursorPosition(engine->GetWindowManager()->GetWindow(), x, y);
	previousCursorPositionForRotating_ = Vector2(x, y);
	isRotatingTheCamera_ = true;
}

void EditorCameraController::OnMouseMiddleClickPressed()
{
	double x, y;
	InputManager::GetCursorPosition(engine->GetWindowManager()->GetWindow(), x, y);
	previousCursorPositionFor2DMovement_ = Vector2(x, y);
	isMovingCameraIn2D_ = true;
}

void EditorCameraController::MoveForward(float multiplier/* = 1.f*/)
{
	activeCamera_ = engine->GetCameraManager()->GetActiveCamera();
	activeCamera_->MoveForward(engine->GetDeltaTime() * multiplier);
}

void EditorCameraController::MoveRight(float multiplier/* = 1.f*/)
{
	activeCamera_ = engine->GetCameraManager()->GetActiveCamera();
	activeCamera_->MoveRight(engine->GetDeltaTime() * multiplier);
}

void EditorCameraController::MoveUp(float multiplier/* = 1.f*/)
{
	activeCamera_ = engine->GetCameraManager()->GetActiveCamera();
	activeCamera_->MoveUpward(engine->GetDeltaTime() * multiplier);
}