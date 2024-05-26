#include "ArcherCharacterController.h"

// Engine includes
#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Managers/InputManager.h"
#include "Goknar/Managers/WindowManager.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Shader.h"

// Game includes
#include "Game.h"
#include "ArcherCharacter.h"
#include "Components/ArcherCharacterMovementComponent.h"
#include "Controllers/FreeCameraController.h"
#include "Objects/FreeCameraObject.h"

ArcherCharacterController::ArcherCharacterController(ArcherCharacter* archer) :
	Controller(),
	archer_(archer)
{
}

void ArcherCharacterController::BeginGame()
{
	archerMovementComponent_ = dynamic_cast<ArcherCharacterMovementComponent*>(archer_->GetMovementComponent());
	thirdPersonCamera_ = archer_->GetThirdPersonCamera();

	engine->GetInputManager()->SetIsCursorVisible(false);
}

void ArcherCharacterController::SetupInputs()
{
	engine->GetInputManager()->AddMouseInputDelegate(MOUSE_MAP::BUTTON_LEFT, INPUT_ACTION::G_PRESS, std::bind(&ArcherCharacterController::DrawBow, this));
	engine->GetInputManager()->AddMouseInputDelegate(MOUSE_MAP::BUTTON_LEFT, INPUT_ACTION::G_RELEASE, std::bind(&ArcherCharacterController::LooseBow, this));

	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::NUM_1, INPUT_ACTION::G_PRESS, std::bind(&ArcherCharacterController::EquipBow, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::G, INPUT_ACTION::G_PRESS, std::bind(&ArcherCharacterController::DropBow, this));
	
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::E, INPUT_ACTION::G_PRESS, std::bind(&ArcherCharacterController::ToggleChest, this));
	//engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::Q, INPUT_ACTION::G_PRESS, std::bind(&ArcherCharacterController::DestroyPhysicsArcher, this));

	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::F1, INPUT_ACTION::G_PRESS, std::bind(&ArcherCharacterController::ToggleDebug, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::F2, INPUT_ACTION::G_PRESS, std::bind(&ArcherCharacterController::ToggleFreeCamera, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::F5, INPUT_ACTION::G_PRESS, std::bind(&ArcherCharacterController::ToggleFullscreen, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::F6, INPUT_ACTION::G_PRESS, std::bind(&ArcherCharacterController::ToggleWindowSize, this));

	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::ESCAPE, INPUT_ACTION::G_PRESS, std::bind(&ArcherCharacterController::ExitGame, this));

	engine->GetInputManager()->AddKeyboardEvent(KEY_MAP::W, this, &ArcherCharacterController::MoveForward, &ArcherCharacterController::StopMovingForward);
	engine->GetInputManager()->AddKeyboardEvent(KEY_MAP::S, this, &ArcherCharacterController::MoveBackward, &ArcherCharacterController::StopMovingBackward);
	engine->GetInputManager()->AddKeyboardEvent(KEY_MAP::A, this, &ArcherCharacterController::MoveLeft, &ArcherCharacterController::StopMovingLeft);
	engine->GetInputManager()->AddKeyboardEvent(KEY_MAP::D, this, &ArcherCharacterController::MoveRight, &ArcherCharacterController::StopMovingRight);
	
	engine->GetInputManager()->AddScrollDelegate(std::bind(&ArcherCharacterController::OnScrollMove, this, std::placeholders::_1, std::placeholders::_2));

	engine->GetInputManager()->AddCursorDelegate(std::bind(&ArcherCharacterController::OnCursorMove, this, std::placeholders::_1, std::placeholders::_2));
}

void ArcherCharacterController::OnCursorMove(double x, double y)
{
	if(isInFreeCamera_) return;

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

void ArcherCharacterController::OnScrollMove(double x, double y)
{
	if(isInFreeCamera_) return;
	if (0 < y)
	{
		archer_->DecreaseThirdPersonCameraDistance();
	}
	else if (y < 0)
	{
		archer_->IncreaseThirdPersonCameraDistance();
	}
}

void ArcherCharacterController::ToggleFullscreen()
{
	engine->GetWindowManager()->ToggleFullscreen();
}

void ArcherCharacterController::ToggleWindowSize()
{
	static bool isWindowSizeSet = false;

	engine->GetWindowManager()->SetWindowSize(isWindowSizeSet ? 1600 : 1200, isWindowSizeSet ? 800 : 600);
	isWindowSizeSet = !isWindowSizeSet;
}

void ArcherCharacterController::ToggleFreeCamera()
{
	Game* game = dynamic_cast<Game*>(engine->GetApplication());

	if(isInFreeCamera_)
	{
		engine->GetCameraManager()->SetActiveCamera(archer_->GetThirdPersonCamera());
		engine->GetInputManager()->SetIsCursorVisible(false);
	}
	else
	{	
		engine->GetCameraManager()->SetActiveCamera(game->GetFreeCameraObject()->GetFreeCameraController()->GetFreeCamera());
		engine->GetInputManager()->SetIsCursorVisible(true);
	}
	
	isInFreeCamera_ = !isInFreeCamera_;
}

void ArcherCharacterController::ExitGame()
{
	engine->Exit();
}

void ArcherCharacterController::DestroyPhysicsArcher()
{
	archer_->Destroy();
}

void ArcherCharacterController::ToggleChest()
{
}

void ArcherCharacterController::DropBow()
{
	if(isInFreeCamera_) return;

	archer_->HandleDropBowInput();
}

void ArcherCharacterController::EquipBow()
{
	if(isInFreeCamera_) return;

	archer_->HandleEquipBowInput();
}

void ArcherCharacterController::DrawBow()
{
	if(isInFreeCamera_) return;

	archer_->HandleDrawBowInput();
}

void ArcherCharacterController::LooseBow()
{
	if(isInFreeCamera_) return;

	archer_->HandleLooseBowInput();
}

void ArcherCharacterController::MoveForward()
{
	if(isInFreeCamera_) return;
	archerMovementComponent_->AddMovementDirection(Vector3::ForwardVector);
}

void ArcherCharacterController::StopMovingForward()
{
	if(isInFreeCamera_) return;
	archerMovementComponent_->AddMovementDirection(-Vector3::ForwardVector);
}

void ArcherCharacterController::MoveBackward()
{
	if (isInFreeCamera_) return;
	archerMovementComponent_->AddMovementDirection(-Vector3::ForwardVector);
}

void ArcherCharacterController::StopMovingBackward()
{
	if (isInFreeCamera_) return;
	archerMovementComponent_->AddMovementDirection(Vector3::ForwardVector);
}

void ArcherCharacterController::MoveLeft()
{
	if (isInFreeCamera_) return;
	archerMovementComponent_->AddMovementDirection(Vector3::LeftVector);
}

void ArcherCharacterController::StopMovingLeft()
{
	if (isInFreeCamera_) return;
	archerMovementComponent_->AddMovementDirection(-Vector3::LeftVector);
}

void ArcherCharacterController::MoveRight()
{
	if (isInFreeCamera_) return;
	archerMovementComponent_->AddMovementDirection(-Vector3::LeftVector);
}

void ArcherCharacterController::StopMovingRight()
{
	if (isInFreeCamera_) return;
	archerMovementComponent_->AddMovementDirection(Vector3::LeftVector);
}

void ArcherCharacterController::ToggleDebug()
{
	isDebugging_ = !isDebugging_;

	DeferredRenderingData* deferredRenderingData = engine->GetRenderer()->GetDeferredRenderingData();
	if(deferredRenderingData)
	{
		deferredRenderingData->deferredRenderingMeshShader->SetBool("isDebugging", isDebugging_);
	}
}