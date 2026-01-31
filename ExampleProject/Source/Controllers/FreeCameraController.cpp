#include "pch.h"

#include "FreeCameraController.h"

#include "Goknar/Scene.h"

#include "Goknar/Application.h"
#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Components/CameraComponent.h"
#include "Goknar/Debug/DebugDrawer.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Managers/InputManager.h"
#include "Goknar/Managers/WindowManager.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Model/MeshUnit.h"
#include "Goknar/Model/IMeshInstance.h"
#include "Goknar/Physics/PhysicsWorld.h"
#include "Goknar/Physics/RigidBody.h"
#include "Goknar/Physics/Components/BoxCollisionComponent.h"

#include "Objects/FreeCameraObject.h"

FreeCameraController::FreeCameraController(FreeCameraObject* freeCameraObject) :
	freeCameraObject_(freeCameraObject),
	isRotatingTheCamera_(false),
	isMovingCameraIn2D_(false)
{
	movementSpeed_ = 50.f;
	previousCursorPositionForRotating_ = Vector2(0.f, 0.f);
	previousCursorPositionFor2DMovement_ = Vector2(0.f, 0.f);

	onMouseRightClickPressedDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::OnMouseRightClickPressed>(this);
	onMouseRightClickReleasedDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::OnMouseRightClickReleased >(this);
	onMouseMiddleClickPressedDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::OnMouseMiddleClickPressed>(this);
	onMouseMiddleClickReleasedDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::OnMouseMiddleClickReleased>(this);

	moveLeftDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::MoveLeftListener>(this);
	moveRightDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::MoveRightListener>(this);
	moveForwardDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::MoveForwardListener>(this);
	moveBackwardDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::MoveBackwardListener>(this);
	moveUpDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::MoveUpListener>(this);
	moveDownDelegate_ = KeyboardDelegate::Create<FreeCameraController, &FreeCameraController::MoveDownListener>(this);

	onScrollMoveDelegate_ = Delegate<void(double, double)>::Create<FreeCameraController, &FreeCameraController::ScrollListener>(this);
	onCursorMoveDelegate_ = Delegate<void(double, double)>::Create<FreeCameraController, &FreeCameraController::CursorMovement>(this);
}

FreeCameraController::~FreeCameraController()
{
	UnbindInputDelegates();
}

void FreeCameraController::BeginGame()
{
	if (GetIsActive())
	{
		BindInputDelegates();
	}
}

void FreeCameraController::SetupInputs()
{
}

void FreeCameraController::SetIsActive(bool isActive)
{
	if (GetIsActive() == isActive)
	{
		return;
	}

	Controller::SetIsActive(isActive);

	if (isActive)
	{
		BindInputDelegates();
	}
	else
	{
		UnbindInputDelegates();
	}
}

void FreeCameraController::CursorMovement(double x, double y)
{
	Vector2 currentCursorPosition(x, y);

	if (isRotatingTheCamera_)
	{
		Vector2 cursorMovementVector = (previousCursorPositionForRotating_ - currentCursorPosition) / 250.f;
		Yaw(cursorMovementVector.x);
		Pitch(-cursorMovementVector.y);

		previousCursorPositionForRotating_ = currentCursorPosition;
	}

	if (isMovingCameraIn2D_)
	{
		Vector2 cursorMovementVector = currentCursorPosition - previousCursorPositionFor2DMovement_;

		MoveLeft(-cursorMovementVector.x);
		MoveUp(-cursorMovementVector.y);

		previousCursorPositionFor2DMovement_ = currentCursorPosition;
	}
}

void FreeCameraController::ScrollListener(double x, double y)
{
	MoveForward(y * movementSpeed_);
}

void FreeCameraController::Yaw(float value)
{
	Vector3 newForwardVector = freeCameraObject_->GetWorldRotation().ToEulerRadians();
	freeCameraObject_->SetWorldRotation(Quaternion::FromEulerRadians(newForwardVector + Vector3{ 0.f, 0.f, value }));
}

void FreeCameraController::Pitch(float value)
{
	Vector3 newForwardVector = freeCameraObject_->GetForwardVector().RotateVectorAroundAxis(freeCameraObject_->GetLeftVector(), value);
	freeCameraObject_->SetWorldRotation(newForwardVector.GetRotationNormalized());
}

void FreeCameraController::OnMouseRightClickPressed()
{
	double x, y;
	engine->GetInputManager()->GetCursorPosition(engine->GetWindowManager()->GetMainWindow(), x, y);
	previousCursorPositionForRotating_ = Vector2(x, y);
	isRotatingTheCamera_ = true;
}

void FreeCameraController::OnMouseMiddleClickPressed()
{
	double x, y;
	engine->GetInputManager()->GetCursorPosition(engine->GetWindowManager()->GetMainWindow(), x, y);
	previousCursorPositionFor2DMovement_ = Vector2(x, y);
	isMovingCameraIn2D_ = true;
}

void FreeCameraController::MoveForward(float multiplier/* = 1.f*/)
{
	freeCameraObject_->SetWorldPosition(
		freeCameraObject_->GetWorldPosition() +
		freeCameraObject_->GetForwardVector() * 0.025f * multiplier);
}

void FreeCameraController::MoveLeft(float multiplier/* = 1.f*/)
{
	freeCameraObject_->SetWorldPosition(
		freeCameraObject_->GetWorldPosition() +
		freeCameraObject_->GetLeftVector() * 0.025f * multiplier);
}

void FreeCameraController::MoveUp(float multiplier/* = 1.f*/)
{
	freeCameraObject_->SetWorldPosition(
		freeCameraObject_->GetWorldPosition() +
		freeCameraObject_->GetUpVector() * 0.025f * multiplier);
}

void FreeCameraController::BindInputDelegates()
{
	InputManager* inputManager = engine->GetInputManager();

	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_PRESS, onMouseRightClickPressedDelegate_);
	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_RELEASE, onMouseRightClickReleasedDelegate_);
	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_PRESS, onMouseMiddleClickPressedDelegate_);
	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_RELEASE, onMouseMiddleClickReleasedDelegate_);

	inputManager->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_REPEAT, moveLeftDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_REPEAT, moveRightDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_REPEAT, moveForwardDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_REPEAT, moveBackwardDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_REPEAT, moveUpDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::LEFT_CONTROL, INPUT_ACTION::G_REPEAT, moveDownDelegate_);

	inputManager->AddScrollDelegate(onScrollMoveDelegate_);
	inputManager->AddCursorDelegate(onCursorMoveDelegate_);

}

void FreeCameraController::UnbindInputDelegates()
{
	InputManager* inputManager = engine->GetInputManager();

	inputManager->RemoveMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_PRESS, onMouseRightClickPressedDelegate_);
	inputManager->RemoveMouseInputDelegate(MOUSE_MAP::BUTTON_RIGHT, INPUT_ACTION::G_RELEASE, onMouseRightClickReleasedDelegate_);
	inputManager->RemoveMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_PRESS, onMouseMiddleClickPressedDelegate_);
	inputManager->RemoveMouseInputDelegate(MOUSE_MAP::BUTTON_MIDDLE, INPUT_ACTION::G_RELEASE, onMouseMiddleClickReleasedDelegate_);

	// Trigger release events to cancel any ongoing operation
	onMouseRightClickReleasedDelegate_();
	onMouseMiddleClickReleasedDelegate_();

	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_REPEAT, moveLeftDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_REPEAT, moveRightDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_REPEAT, moveForwardDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_REPEAT, moveBackwardDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_REPEAT, moveUpDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::LEFT_CONTROL, INPUT_ACTION::G_REPEAT, moveDownDelegate_);

	inputManager->RemoveScrollDelegate(onScrollMoveDelegate_);
	inputManager->RemoveCursorDelegate(onCursorMoveDelegate_);
}
