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
#include "Components/ArcherPhysicsMovementComponent.h"
#include "Controllers/FreeCameraController.h"
#include "Objects/FreeCameraObject.h"

ArcherCharacterController::ArcherCharacterController(ArcherCharacter* archer) :
	Controller(),
	archer_(archer)
{
	moveForwardDelegate_ = KeyboardDelegate::create<ArcherCharacterController, &ArcherCharacterController::MoveForward>(this);
	stopMovingForwardDelegate_ = KeyboardDelegate::create<ArcherCharacterController, &ArcherCharacterController::StopMovingForward>(this);
	moveBackwardDelegate_ = KeyboardDelegate::create<ArcherCharacterController, &ArcherCharacterController::MoveBackward>(this);
	stopMovingBackwardDelegate_ = KeyboardDelegate::create<ArcherCharacterController, &ArcherCharacterController::StopMovingBackward>(this);
	moveLeftDelegate_ = KeyboardDelegate::create<ArcherCharacterController, &ArcherCharacterController::MoveLeft>(this);
	stopMovingLeftDelegate_ = KeyboardDelegate::create<ArcherCharacterController, &ArcherCharacterController::StopMovingLeft>(this);
	moveRightDelegate_ = KeyboardDelegate::create<ArcherCharacterController, &ArcherCharacterController::MoveRight>(this);
	stopMovingRightDelegate_ = KeyboardDelegate::create<ArcherCharacterController, &ArcherCharacterController::StopMovingRight>(this);
	toggleDebugDelegate_ = KeyboardDelegate::create<ArcherCharacterController, &ArcherCharacterController::ToggleDebug>(this);
	toggleToggleFreeCameraDelegate_ = KeyboardDelegate::create<ArcherCharacterController, &ArcherCharacterController::ToggleFreeCamera>(this);
	onScrollMoveDelegate_ = Delegate<void(double, double)>::create<ArcherCharacterController, &ArcherCharacterController::OnScrollMove>(this);
	onCursorMoveDelegate_ = Delegate<void(double, double)>::create<ArcherCharacterController, &ArcherCharacterController::OnCursorMove>(this);
}

ArcherCharacterController::~ArcherCharacterController()
{
	UnbindInputDelegates();
}

void ArcherCharacterController::BeginGame()
{
	archerMovementComponent_ = dynamic_cast<ArcherPhysicsMovementComponent*>(archer_->GetMovementComponent());
	thirdPersonCamera_ = archer_->GetThirdPersonCamera();

	if (GetIsActive())
	{
		BindInputDelegates();
	}
}

void ArcherCharacterController::SetupInputs()
{
}

void ArcherCharacterController::SetIsActive(bool isActive)
{
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

void ArcherCharacterController::BindInputDelegates()
{
	InputManager* inputManager = engine->GetInputManager();

	inputManager->SetIsCursorVisible(false);

	inputManager->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_PRESS, moveForwardDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_RELEASE, stopMovingForwardDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_PRESS, moveBackwardDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_RELEASE, stopMovingBackwardDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_PRESS, moveLeftDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_RELEASE, stopMovingLeftDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_PRESS, moveRightDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_RELEASE, stopMovingRightDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::F1, INPUT_ACTION::G_RELEASE, toggleDebugDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::F2, INPUT_ACTION::G_RELEASE, toggleToggleFreeCameraDelegate_);

	inputManager->AddScrollDelegate(onScrollMoveDelegate_);
	inputManager->AddCursorDelegate(onCursorMoveDelegate_);
}

void ArcherCharacterController::UnbindInputDelegates()
{
	InputManager* inputManager = engine->GetInputManager();

	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_PRESS, moveForwardDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_RELEASE, stopMovingForwardDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_PRESS, moveBackwardDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_RELEASE, stopMovingBackwardDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_PRESS, moveLeftDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_RELEASE, stopMovingLeftDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_PRESS, moveRightDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_RELEASE, stopMovingRightDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::F1, INPUT_ACTION::G_RELEASE, toggleDebugDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::F2, INPUT_ACTION::G_RELEASE, toggleToggleFreeCameraDelegate_);

	inputManager->RemoveScrollDelegate(onScrollMoveDelegate_);
	inputManager->RemoveCursorDelegate(onCursorMoveDelegate_);
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