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
	moveForwardDelegate_ = KeyboardDelegate::create<ArcherGameController, &ArcherGameController::MoveForward>(this);
	stopMovingForwardDelegate_ = KeyboardDelegate::create<ArcherGameController, &ArcherGameController::StopMovingForward>(this);
	moveBackwardDelegate_ = KeyboardDelegate::create<ArcherGameController, &ArcherGameController::MoveBackward>(this);
	stopMovingBackwardDelegate_ = KeyboardDelegate::create<ArcherGameController, &ArcherGameController::StopMovingBackward>(this);
	moveLeftDelegate_ = KeyboardDelegate::create<ArcherGameController, &ArcherGameController::MoveLeft>(this);
	stopMovingLeftDelegate_ = KeyboardDelegate::create<ArcherGameController, &ArcherGameController::StopMovingLeft>(this);
	moveRightDelegate_ = KeyboardDelegate::create<ArcherGameController, &ArcherGameController::MoveRight>(this);
	stopMovingRightDelegate_ = KeyboardDelegate::create<ArcherGameController, &ArcherGameController::StopMovingRight>(this);
	onScrollMoveDelegate_ = Delegate<void(double, double)>::create<ArcherGameController, &ArcherGameController::OnScrollMove>(this);
	onCursorMoveDelegate_ = Delegate<void(double, double)>::create<ArcherGameController, &ArcherGameController::OnCursorMove>(this);
}

ArcherGameController::~ArcherGameController()
{
	//InputManager* inputManager = engine->GetInputManager();

	//inputManager->RemoveKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_PRESS, moveForwardDelegate_);
	//inputManager->RemoveKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_RELEASE, stopMovingForwardDelegate_);
	//inputManager->RemoveKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_PRESS, moveBackwardDelegate_);
	//inputManager->RemoveKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_RELEASE, stopMovingBackwardDelegate_);
	//inputManager->RemoveKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_PRESS, moveLeftDelegate_);
	//inputManager->RemoveKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_RELEASE, stopMovingLeftDelegate_);
	//inputManager->RemoveKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_PRESS, moveRightDelegate_);
	//inputManager->RemoveKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_RELEASE, stopMovingRightDelegate_);

	//inputManager->RemoveScrollDelegate(onScrollMoveDelegate_);
	//inputManager->RemoveCursorDelegate(onCursorMoveDelegate_);
}

void ArcherGameController::BeginGame()
{
	archerMovementComponent_ = archer_->GetMovementComponent();
	thirdPersonCamera_ = archer_->GetThirdPersonCamera();

	//InputManager* inputManager = engine->GetInputManager();

	//inputManager->SetIsCursorVisible(false);

	//inputManager->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_PRESS, moveForwardDelegate_);
	//inputManager->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_RELEASE, stopMovingForwardDelegate_);
	//inputManager->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_PRESS, moveBackwardDelegate_);
	//inputManager->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_RELEASE, stopMovingBackwardDelegate_);
	//inputManager->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_PRESS, moveLeftDelegate_);
	//inputManager->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_RELEASE, stopMovingLeftDelegate_);
	//inputManager->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_PRESS, moveRightDelegate_);
	//inputManager->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_RELEASE, stopMovingRightDelegate_);

	//inputManager->AddScrollDelegate(onScrollMoveDelegate_);
	//inputManager->AddCursorDelegate(onCursorMoveDelegate_);
}

void ArcherGameController::SetupInputs()
{
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
