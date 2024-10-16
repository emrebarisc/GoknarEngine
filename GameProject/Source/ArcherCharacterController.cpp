#include "ArcherCharacterController.h"

// Engine includes
#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "Goknar/Components/CameraComponent.h"
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
	moveForwardDelegate_ = KeyboardDelegate::Create<ArcherCharacterController, &ArcherCharacterController::MoveForward>(this);
	stopMovingForwardDelegate_ = KeyboardDelegate::Create<ArcherCharacterController, &ArcherCharacterController::StopMovingForward>(this);
	moveBackwardDelegate_ = KeyboardDelegate::Create<ArcherCharacterController, &ArcherCharacterController::MoveBackward>(this);
	stopMovingBackwardDelegate_ = KeyboardDelegate::Create<ArcherCharacterController, &ArcherCharacterController::StopMovingBackward>(this);
	moveLeftDelegate_ = KeyboardDelegate::Create<ArcherCharacterController, &ArcherCharacterController::MoveLeft>(this);
	stopMovingLeftDelegate_ = KeyboardDelegate::Create<ArcherCharacterController, &ArcherCharacterController::StopMovingLeft>(this);
	moveRightDelegate_ = KeyboardDelegate::Create<ArcherCharacterController, &ArcherCharacterController::MoveRight>(this);
	stopMovingRightDelegate_ = KeyboardDelegate::Create<ArcherCharacterController, &ArcherCharacterController::StopMovingRight>(this);
	toggleDebugDelegate_ = KeyboardDelegate::Create<ArcherCharacterController, &ArcherCharacterController::ToggleDebug>(this);
	toggleToggleFreeCameraDelegate_ = KeyboardDelegate::Create<ArcherCharacterController, &ArcherCharacterController::ToggleFreeCamera>(this);

	dropBowDelegate_ = KeyboardDelegate::Create<ArcherCharacterController, &ArcherCharacterController::DropBow>(this);
	equipBowDelegate_ = KeyboardDelegate::Create<ArcherCharacterController, &ArcherCharacterController::EquipBow>(this);
	drawBowDelegate_ = KeyboardDelegate::Create<ArcherCharacterController, &ArcherCharacterController::DrawBow>(this);
	looseBowDelegate_ = KeyboardDelegate::Create<ArcherCharacterController, &ArcherCharacterController::LooseBow>(this);

	onScrollMoveDelegate_ = Delegate<void(double, double)>::Create<ArcherCharacterController, &ArcherCharacterController::OnScrollMove>(this);
	onCursorMoveDelegate_ = Delegate<void(double, double)>::Create<ArcherCharacterController, &ArcherCharacterController::OnCursorMove>(this);

	switchToFreeCameraDelegate_ = KeyboardDelegate::Create<ArcherCharacterController, &ArcherCharacterController::SwitchToFreeCamera>(this);

	exitGameDelegate_ = Delegate<void()>::Create<Engine, &Engine::Exit>(engine);
}

ArcherCharacterController::~ArcherCharacterController()
{
	UnbindInputDelegates();
}

void ArcherCharacterController::BeginGame()
{
	archerMovementComponent_ = dynamic_cast<ArcherPhysicsMovementComponent*>(archer_->GetMovementComponent());

	if (GetIsActive())
	{
		BindInputDelegates();
	}

	thirdPersonCameraComponent_ = archer_->GetThirdPersonCameraComponent();
	SetNewCameraPosition();
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
		engine->GetCameraManager()->SetActiveCamera(thirdPersonCameraComponent_->GetCamera());
		engine->GetInputManager()->SetIsCursorVisible(false);
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
	inputManager->AddKeyboardInputDelegate(KEY_MAP::ESCAPE, INPUT_ACTION::G_PRESS, exitGameDelegate_);

	inputManager->AddKeyboardInputDelegate(KEY_MAP::G, INPUT_ACTION::G_PRESS, dropBowDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::NUM_1, INPUT_ACTION::G_PRESS, equipBowDelegate_);
	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_1, INPUT_ACTION::G_PRESS, drawBowDelegate_);
	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_1, INPUT_ACTION::G_RELEASE, looseBowDelegate_);

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
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::ESCAPE, INPUT_ACTION::G_PRESS, exitGameDelegate_);

	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::G, INPUT_ACTION::G_PRESS, dropBowDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::NUM_1, INPUT_ACTION::G_PRESS, equipBowDelegate_);
	inputManager->RemoveMouseInputDelegate(MOUSE_MAP::BUTTON_1, INPUT_ACTION::G_PRESS, drawBowDelegate_);
	inputManager->RemoveMouseInputDelegate(MOUSE_MAP::BUTTON_1, INPUT_ACTION::G_RELEASE, looseBowDelegate_);

	inputManager->RemoveScrollDelegate(onScrollMoveDelegate_);
	inputManager->RemoveCursorDelegate(onCursorMoveDelegate_);
}

void ArcherCharacterController::OnCursorMove(double x, double y)
{
	Vector2i windowSize = engine->GetWindowManager()->GetWindowSize();
	Vector2 windowCenter = windowSize * 0.5f;
	Vector2 currentCursorPosition{ (float)x, (float)y };

	Vector2 cursorMovementVector = (windowCenter - currentCursorPosition) / 1000.f;

	Vector3 forwardVector = thirdPersonCameraComponent_->GetRelativeForwardVector();

	Vector3 newForwardVector = forwardVector.RotateVectorAroundAxis(Vector3::UpVector, cursorMovementVector.x);
	Vector3 newLeftVector = Vector3::Cross(Vector3::UpVector, newForwardVector);

	if ((forwardVector.z < 0.25f && 0.f < cursorMovementVector.y) ||
		(-0.9f < forwardVector.z && cursorMovementVector.y < 0.f))
	{
		newForwardVector = newForwardVector.RotateVectorAroundAxis(thirdPersonCameraComponent_->GetRelativeLeftVector(), -cursorMovementVector.y);
	}

	thirdPersonCameraComponent_->SetRelativeRotation(newForwardVector.GetRotationNormalized());

	SetNewCameraPosition();

	engine->GetInputManager()->SetCursorPosition(windowCenter.x, windowCenter.y);
}

void ArcherCharacterController::OnScrollMove(double x, double y)
{
	if (0 < y)
	{
		DecreaseThirdPersonCameraDistance();
	}
	else if (y < 0)
	{
		IncreaseThirdPersonCameraDistance();
	}
}

void ArcherCharacterController::SetNewCameraPosition()
{
	thirdPersonCameraComponent_->SetRelativePosition(Vector3(0.f, 0.f, 2.f) + thirdPersonCameraComponent_->GetCamera()->GetForwardVector() * -4.f * thirdPersonCameraDistance_);
}

void ArcherCharacterController::IncreaseThirdPersonCameraDistance()
{
	if (thirdPersonCameraDistance_ < 5.f)
	{
		thirdPersonCameraDistance_ += 0.25f;
		SetNewCameraPosition();
	}
}

void ArcherCharacterController::DecreaseThirdPersonCameraDistance()
{
	if (0.25f < thirdPersonCameraDistance_)
	{
		thirdPersonCameraDistance_ -= 0.25f;
		SetNewCameraPosition();
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
	archer_->HandleDropBowInput();
}

void ArcherCharacterController::EquipBow()
{
	archer_->HandleEquipBowInput();
}

void ArcherCharacterController::DrawBow()
{
	archer_->HandleDrawBowInput();
}

void ArcherCharacterController::LooseBow()
{
	archer_->HandleLooseBowInput();
}

void ArcherCharacterController::MoveForward()
{	
	archerMovementComponent_->AddMovementDirection(Vector3::ForwardVector);
}

void ArcherCharacterController::StopMovingForward()
{	
	archerMovementComponent_->AddMovementDirection(-Vector3::ForwardVector);
}

void ArcherCharacterController::MoveBackward()
{
	archerMovementComponent_->AddMovementDirection(-Vector3::ForwardVector);
}

void ArcherCharacterController::StopMovingBackward()
{
	archerMovementComponent_->AddMovementDirection(Vector3::ForwardVector);
}

void ArcherCharacterController::MoveLeft()
{
	archerMovementComponent_->AddMovementDirection(Vector3::LeftVector);
}

void ArcherCharacterController::StopMovingLeft()
{
	archerMovementComponent_->AddMovementDirection(-Vector3::LeftVector);
}

void ArcherCharacterController::MoveRight()
{
	archerMovementComponent_->AddMovementDirection(-Vector3::LeftVector);
}

void ArcherCharacterController::StopMovingRight()
{
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