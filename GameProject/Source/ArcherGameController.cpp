#include "ArcherGameController.h"

// Engine includes
#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/InputManager.h"
#include "Goknar/Managers/WindowManager.h"

// Game includes
#include "Archer.h"
#include "Components/ArcherMovementComponent.h"

#define MAX_BONE_INDEX 37
#define MAX_ANIMATION_INDEX 19

ArcherGameController::ArcherGameController(Archer* archer) :
	Controller(),
	archer_(archer)
{
}

void ArcherGameController::BeginGame()
{
	archerMovementComponent_ = archer_->GetMovementComponent();
	thirdPersonCamera_ = archer_->GetThirdPersonCamera();

	engine->GetInputManager()->SetIsCursorVisible(false);
}

void ArcherGameController::SetupInputs()
{
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::NUM_1, INPUT_ACTION::G_PRESS, std::bind(&ArcherGameController::EquipBow, this));

	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::G, INPUT_ACTION::G_PRESS, std::bind(&ArcherGameController::DropBow, this));

	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_PRESS, std::bind(&ArcherGameController::MoveForward, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_RELEASE, std::bind(&ArcherGameController::StopMovingForward, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_PRESS, std::bind(&ArcherGameController::MoveBackward, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_RELEASE, std::bind(&ArcherGameController::StopMovingBackward, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_PRESS, std::bind(&ArcherGameController::MoveLeft, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_RELEASE, std::bind(&ArcherGameController::StopMovingLeft, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_PRESS, std::bind(&ArcherGameController::MoveRight, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_RELEASE, std::bind(&ArcherGameController::StopMovingRight, this));

	engine->GetInputManager()->AddScrollDelegate(std::bind(&ArcherGameController::OnScrollMove, this, std::placeholders::_1, std::placeholders::_2));

	engine->GetInputManager()->AddCursorDelegate(std::bind(&ArcherGameController::OnCursorMove, this, std::placeholders::_1, std::placeholders::_2));
}

void ArcherGameController::OnCursorMove(double x, double y)
{
	Vector2i windowSize = engine->GetWindowManager()->GetWindowSize();
	Vector2 windowCenter = windowSize * 0.5f;
	Vector2 currentCursorPosition{(float)x, (float)y};

	Vector2 cursorMovementVector = (currentCursorPosition - windowCenter) / 1000.f;

	thirdPersonCamera_->RotateAbout(Vector3::UpVector, -cursorMovementVector.x);

	if ((thirdPersonCamera_->GetForwardVector().z < 0.25f && 0.f < cursorMovementVector.y) ||
		(-0.9f < thirdPersonCamera_->GetForwardVector().z && cursorMovementVector.y < 0.f))
	{
		thirdPersonCamera_->Pitch(cursorMovementVector.y);
	}

	engine->GetInputManager()->SetCursorPosition(windowCenter.x, windowCenter.y);
}

void ArcherGameController::OnScrollMove(double x, double y)
{
	if (0 < y)
	{
		archer_->DecreaseThirdPersonCameraDistance();
	}
	else if (y < 0)
	{
		archer_->IncreaseThirdPersonCameraDistance();
	}
}

void ArcherGameController::DropBow()
{
	archer_->HandleDropBowInput();
}

void ArcherGameController::EquipBow()
{
	archer_->HandleEquipBowInput();
}

void ArcherGameController::MoveForward()
{
	archerMovementComponent_->AddMovementVector(Vector3::ForwardVector);
}

void ArcherGameController::StopMovingForward()
{
	archerMovementComponent_->AddMovementVector(-Vector3::ForwardVector);
}

void ArcherGameController::MoveBackward()
{
	archerMovementComponent_->AddMovementVector(-Vector3::ForwardVector);
}

void ArcherGameController::StopMovingBackward()
{
	archerMovementComponent_->AddMovementVector(Vector3::ForwardVector);
}

void ArcherGameController::MoveLeft()
{
	archerMovementComponent_->AddMovementVector(Vector3::LeftVector);
}

void ArcherGameController::StopMovingLeft()
{
	archerMovementComponent_->AddMovementVector(-Vector3::LeftVector);
}

void ArcherGameController::MoveRight()
{
	archerMovementComponent_->AddMovementVector(-Vector3::LeftVector);
}

void ArcherGameController::StopMovingRight()
{
	archerMovementComponent_->AddMovementVector(Vector3::LeftVector);
}