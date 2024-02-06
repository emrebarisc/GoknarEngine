#include "PhysicsArcherGameController.h"

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
#include "PhysicsArcher.h"
#include "Components/PhysicsArcherMovementComponent.h"
#include "Controllers/FreeCameraController.h"
#include "Objects/FreeCameraObject.h"

#define MAX_BONE_INDEX 37
#define MAX_ANIMATION_INDEX 19

PhysicsArcherGameController::PhysicsArcherGameController(PhysicsArcher* archer) :
	Controller(),
	archer_(archer)
{
}

void PhysicsArcherGameController::BeginGame()
{
	archerMovementComponent_ = archer_->GetMovementComponent();
	thirdPersonCamera_ = archer_->GetThirdPersonCamera();

	engine->GetInputManager()->SetIsCursorVisible(false);
}


void PhysicsArcherGameController::SetupInputs()
{
	engine->GetInputManager()->AddMouseInputDelegate(MOUSE_MAP::BUTTON_1, INPUT_ACTION::G_PRESS, std::bind(&PhysicsArcherGameController::DrawBow, this));
	engine->GetInputManager()->AddMouseInputDelegate(MOUSE_MAP::BUTTON_1, INPUT_ACTION::G_RELEASE, std::bind(&PhysicsArcherGameController::LooseBow, this));

	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::NUM_1, INPUT_ACTION::G_PRESS, std::bind(&PhysicsArcherGameController::EquipBow, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::G, INPUT_ACTION::G_PRESS, std::bind(&PhysicsArcherGameController::DropBow, this));
	
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::E, INPUT_ACTION::G_PRESS, std::bind(&PhysicsArcherGameController::ToggleChest, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::Q, INPUT_ACTION::G_PRESS, std::bind(&PhysicsArcherGameController::DestroyPhysicsArcher, this));

	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::F1, INPUT_ACTION::G_PRESS, std::bind(&PhysicsArcherGameController::ToggleDebug, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::F2, INPUT_ACTION::G_PRESS, std::bind(&PhysicsArcherGameController::ToggleFreeCamera, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::F5, INPUT_ACTION::G_PRESS, std::bind(&PhysicsArcherGameController::ToggleFullscreen, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::F6, INPUT_ACTION::G_PRESS, std::bind(&PhysicsArcherGameController::ToggleWindowSize, this));

	engine->GetInputManager()->AddKeyboardEvent(KEY_MAP::W, this, &PhysicsArcherGameController::MoveForward, &PhysicsArcherGameController::StopMovingForward);
	engine->GetInputManager()->AddKeyboardEvent(KEY_MAP::S, this, &PhysicsArcherGameController::MoveBackward, &PhysicsArcherGameController::StopMovingBackward);
	engine->GetInputManager()->AddKeyboardEvent(KEY_MAP::A, this, &PhysicsArcherGameController::MoveLeft, &PhysicsArcherGameController::StopMovingLeft);
	engine->GetInputManager()->AddKeyboardEvent(KEY_MAP::D, this, &PhysicsArcherGameController::MoveRight, &PhysicsArcherGameController::StopMovingRight);
	
	engine->GetInputManager()->AddScrollDelegate(std::bind(&PhysicsArcherGameController::OnScrollMove, this, std::placeholders::_1, std::placeholders::_2));

	engine->GetInputManager()->AddCursorDelegate(std::bind(&PhysicsArcherGameController::OnCursorMove, this, std::placeholders::_1, std::placeholders::_2));
}

void PhysicsArcherGameController::OnCursorMove(double x, double y)
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

void PhysicsArcherGameController::OnScrollMove(double x, double y)
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

void PhysicsArcherGameController::ToggleFullscreen()
{
	engine->GetWindowManager()->ToggleFullscreen();
}

void PhysicsArcherGameController::ToggleWindowSize()
{
	static bool isWindowSizeSet = false;

	engine->GetWindowManager()->SetWindowSize(isWindowSizeSet ? 1600 : 1200, isWindowSizeSet ? 800 : 600);
	isWindowSizeSet = !isWindowSizeSet;
}

void PhysicsArcherGameController::ToggleFreeCamera()
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

void PhysicsArcherGameController::DestroyPhysicsArcher()
{
	archer_->Destroy();
}

void PhysicsArcherGameController::ToggleChest()
{
}

void PhysicsArcherGameController::DropBow()
{
	archer_->HandleDropBowInput();
}

void PhysicsArcherGameController::EquipBow()
{
	archer_->HandleEquipBowInput();
}

void PhysicsArcherGameController::DrawBow()
{
	archer_->HandleDrawBowInput();
}

void PhysicsArcherGameController::LooseBow()
{
	archer_->HandleLooseBowInput();
}

void PhysicsArcherGameController::MoveForward()
{
	if(isInFreeCamera_) return;
	archerMovementComponent_->AddMovementVector(Vector3::ForwardVector);
}

void PhysicsArcherGameController::StopMovingForward()
{
	if(isInFreeCamera_) return;
	archerMovementComponent_->AddMovementVector(-Vector3::ForwardVector);
}

void PhysicsArcherGameController::MoveBackward()
{
	if(isInFreeCamera_) return;
	archerMovementComponent_->AddMovementVector(-Vector3::ForwardVector);
}

void PhysicsArcherGameController::StopMovingBackward()
{
	if(isInFreeCamera_) return;
	archerMovementComponent_->AddMovementVector(Vector3::ForwardVector);
}

void PhysicsArcherGameController::MoveLeft()
{
	if(isInFreeCamera_) return;
	archerMovementComponent_->AddMovementVector(Vector3::LeftVector);
}

void PhysicsArcherGameController::StopMovingLeft()
{
	if(isInFreeCamera_) return;
	archerMovementComponent_->AddMovementVector(-Vector3::LeftVector);
}

void PhysicsArcherGameController::MoveRight()
{
	if(isInFreeCamera_) return;
	archerMovementComponent_->AddMovementVector(-Vector3::LeftVector);
}

void PhysicsArcherGameController::StopMovingRight()
{
	if(isInFreeCamera_) return;
	archerMovementComponent_->AddMovementVector(Vector3::LeftVector);
}

void PhysicsArcherGameController::ToggleDebug()
{
	isDebugging_ = !isDebugging_;

	DeferredRenderingData* deferredRenderingData = engine->GetRenderer()->GetDeferredRenderingData();
	if(deferredRenderingData)
	{
		deferredRenderingData->deferredRenderingMeshShader->SetBool("isDebugging", isDebugging_);
	}
}
