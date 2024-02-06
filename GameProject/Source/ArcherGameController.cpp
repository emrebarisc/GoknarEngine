#include "ArcherGameController.h"

// Engine includes
#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/InputManager.h"
#include "Goknar/Managers/WindowManager.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Shader.h"

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
	engine->GetInputManager()->AddMouseInputDelegate(MOUSE_MAP::BUTTON_1, INPUT_ACTION::G_PRESS, std::bind(&ArcherGameController::DrawBow, this));
	engine->GetInputManager()->AddMouseInputDelegate(MOUSE_MAP::BUTTON_1, INPUT_ACTION::G_RELEASE, std::bind(&ArcherGameController::LooseBow, this));

	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::NUM_1, INPUT_ACTION::G_PRESS, std::bind(&ArcherGameController::EquipBow, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::G, INPUT_ACTION::G_PRESS, std::bind(&ArcherGameController::DropBow, this));
	
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::F5, INPUT_ACTION::G_PRESS, std::bind(&ArcherGameController::ToggleFullscreen, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::F6, INPUT_ACTION::G_PRESS, std::bind(&ArcherGameController::ToggleWindowSize, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::E, INPUT_ACTION::G_PRESS, std::bind(&ArcherGameController::ToggleChest, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::Q, INPUT_ACTION::G_PRESS, std::bind(&ArcherGameController::DestroyArcher, this));

	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::F1, INPUT_ACTION::G_PRESS, std::bind(&ArcherGameController::ToggleDebug, this));

	engine->GetInputManager()->AddKeyboardEvent(KEY_MAP::W, this, &ArcherGameController::MoveForward, &ArcherGameController::StopMovingForward);
	engine->GetInputManager()->AddKeyboardEvent(KEY_MAP::S, this, &ArcherGameController::MoveBackward, &ArcherGameController::StopMovingBackward);
	engine->GetInputManager()->AddKeyboardEvent(KEY_MAP::A, this, &ArcherGameController::MoveLeft, &ArcherGameController::StopMovingLeft);
	engine->GetInputManager()->AddKeyboardEvent(KEY_MAP::D, this, &ArcherGameController::MoveRight, &ArcherGameController::StopMovingRight);
	
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

void ArcherGameController::ToggleFullscreen()
{
	engine->GetWindowManager()->ToggleFullscreen();
}

void ArcherGameController::ToggleWindowSize()
{
	static bool isWindowSizeSet = false;

	engine->GetWindowManager()->SetWindowSize(isWindowSizeSet ? 1600 : 1200, isWindowSizeSet ? 800 : 600);
	isWindowSizeSet = !isWindowSizeSet;
}

void ArcherGameController::DestroyArcher()
{
	archer_->Destroy();
}

void ArcherGameController::ToggleChest()
{
}

void ArcherGameController::DropBow()
{
	archer_->HandleDropBowInput();
}

void ArcherGameController::EquipBow()
{
	archer_->HandleEquipBowInput();
}

void ArcherGameController::DrawBow()
{
	archer_->HandleDrawBowInput();
}

void ArcherGameController::LooseBow()
{
	archer_->HandleLooseBowInput();
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

void ArcherGameController::ToggleDebug()
{
	isDebugging_ = !isDebugging_;

	DeferredRenderingData* deferredRenderingData = engine->GetRenderer()->GetDeferredRenderingData();
	if(deferredRenderingData)
	{
		deferredRenderingData->deferredRenderingMeshShader->SetBool("isDebugging", isDebugging_);
	}
}
