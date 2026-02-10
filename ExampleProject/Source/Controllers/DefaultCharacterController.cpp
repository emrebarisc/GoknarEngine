#include "DefaultCharacterController.h"

#include "Goknar/Engine.h"
#include "Goknar/Managers/InputManager.h"
#include "Goknar/Managers/WindowManager.h"

#include "Game.h"
#include "GameState.h"
#include "Characters/DefaultCharacter.h"
#include "Components/DefaultCharacterMovementComponent.h"

DefaultCharacterController::DefaultCharacterController(DefaultCharacter* character) :
	Controller(),
	character_(character)
{
	SetIsTickable(true);
	SetName("DefaultCharacterController");

	moveForwardDelegate_ = KeyboardDelegate::Create<DefaultCharacterController, &DefaultCharacterController::MoveForward>(this);
	stopMovingForwardDelegate_ = KeyboardDelegate::Create<DefaultCharacterController, &DefaultCharacterController::StopMovingForward>(this);
	moveBackwardDelegate_ = KeyboardDelegate::Create<DefaultCharacterController, &DefaultCharacterController::MoveBackward>(this);
	stopMovingBackwardDelegate_ = KeyboardDelegate::Create<DefaultCharacterController, &DefaultCharacterController::StopMovingBackward>(this);
	moveLeftDelegate_ = KeyboardDelegate::Create<DefaultCharacterController, &DefaultCharacterController::MoveLeft>(this);
	stopMovingLeftDelegate_ = KeyboardDelegate::Create<DefaultCharacterController, &DefaultCharacterController::StopMovingLeft>(this);
	moveRightDelegate_ = KeyboardDelegate::Create<DefaultCharacterController, &DefaultCharacterController::MoveRight>(this);
	stopMovingRightDelegate_ = KeyboardDelegate::Create<DefaultCharacterController, &DefaultCharacterController::StopMovingRight>(this);
	startRunningDelegate_= KeyboardDelegate::Create<DefaultCharacterController, &DefaultCharacterController::StartRunning>(this);
	stopRunningDelegate_ = KeyboardDelegate::Create<DefaultCharacterController, &DefaultCharacterController::StopRunning>(this);
	jumpDelegate_ = KeyboardDelegate::Create<DefaultCharacterController, &DefaultCharacterController::Jump>(this);
	fireDelegate_ = MouseDelegate::Create<DefaultCharacterController, &DefaultCharacterController::Fire>(this);

	onPauseDelegate_ = Delegate<void()>::Create<DefaultCharacterController, &DefaultCharacterController::OnPause>(this);

	cursorDelegate_ = Delegate<void(double, double)>::Create<DefaultCharacterController, &DefaultCharacterController::OnCursorMove>(this);
}

DefaultCharacterController::~DefaultCharacterController()
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
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::LEFT_SHIFT, INPUT_ACTION::G_PRESS, startRunningDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::LEFT_SHIFT, INPUT_ACTION::G_RELEASE, stopRunningDelegate_);
	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_PRESS, jumpDelegate_);

	inputManager->RemoveKeyboardInputDelegate(KEY_MAP::ESCAPE, INPUT_ACTION::G_PRESS, onPauseDelegate_);

	inputManager->RemoveMouseInputDelegate(MOUSE_MAP::BUTTON_LEFT, INPUT_ACTION::G_PRESS, fireDelegate_);
	
	inputManager->RemoveCursorDelegate(cursorDelegate_);
}

void DefaultCharacterController::BeginGame()
{
	characterMovementComponent_ = dynamic_cast<DefaultCharacterMovementComponent*>(character_->GetMovementComponent());

	InputManager* inputManager = engine->GetInputManager();
	WindowManager* windowManager = engine->GetWindowManager();
	Vector2i windowSize = windowManager->GetWindowSize();
	Vector2 windowCenter = windowSize * 0.5f;
	engine->GetInputManager()->SetCursorPosition(windowCenter.x, windowCenter.y);

	inputManager->SetIsCursorVisible(false);

	onGround_ = characterMovementComponent_->OnGround();
}

void DefaultCharacterController::Tick(float deltaTime)
{
	Controller::Tick(deltaTime);

	bool newOnGround = characterMovementComponent_->OnGround();

	if (onGround_ != newOnGround)
	{
		if (newOnGround)
		{
			character_->OnGround();
		}
		else
		{
			character_->OnAir();
		}
		onGround_ = newOnGround;
	}
}

void DefaultCharacterController::SetupInputs()
{
	InputManager* inputManager = engine->GetInputManager();
	inputManager->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_PRESS, moveForwardDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_RELEASE, stopMovingForwardDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_PRESS, moveBackwardDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_RELEASE, stopMovingBackwardDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_PRESS, moveLeftDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_RELEASE, stopMovingLeftDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_PRESS, moveRightDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_RELEASE, stopMovingRightDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::LEFT_SHIFT, INPUT_ACTION::G_PRESS, startRunningDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::LEFT_SHIFT, INPUT_ACTION::G_RELEASE, stopRunningDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_RELEASE, jumpDelegate_);
	inputManager->AddKeyboardInputDelegate(KEY_MAP::ESCAPE, INPUT_ACTION::G_PRESS, onPauseDelegate_);

	inputManager->AddMouseInputDelegate(MOUSE_MAP::BUTTON_LEFT, INPUT_ACTION::G_PRESS, fireDelegate_);

	inputManager->AddKeyboardInputDelegate(KEY_MAP::F5, INPUT_ACTION::G_PRESS, []() { engine->SetTimeScale(0.f); });
	inputManager->AddKeyboardInputDelegate(KEY_MAP::F6, INPUT_ACTION::G_PRESS, []() { engine->SetTimeScale(1.f); });
	inputManager->AddKeyboardInputDelegate(KEY_MAP::F7, INPUT_ACTION::G_PRESS, []() { float newTimeScale = GoknarMath::Max(engine->GetTimeScale() - 0.1f, 0.f); engine->SetTimeScale(newTimeScale); });
	inputManager->AddKeyboardInputDelegate(KEY_MAP::F8, INPUT_ACTION::G_PRESS, []() { float newTimeScale = GoknarMath::Min(engine->GetTimeScale() + 0.1f, 10.f); engine->SetTimeScale(newTimeScale); });

	inputManager->AddCursorDelegate(cursorDelegate_);
}

void DefaultCharacterController::MoveForward()
{
	if(!moveForward_)
	{
		characterMovementComponent_->AddMovementDirection(Vector3::ForwardVector);
		moveForward_ = true;
	}
}

void DefaultCharacterController::StopMovingForward()
{
	if(moveForward_)
	{
		characterMovementComponent_->AddMovementDirection(-Vector3::ForwardVector);
		moveForward_ = false;
	}
}

void DefaultCharacterController::MoveBackward()
{
	if(!moveBackward_)
	{
		characterMovementComponent_->AddMovementDirection(-Vector3::ForwardVector);
		moveBackward_ = true;
	}
}

void DefaultCharacterController::StopMovingBackward()
{
	if(moveBackward_)
	{
		characterMovementComponent_->AddMovementDirection(Vector3::ForwardVector);
		moveBackward_ = false;
	}
}

void DefaultCharacterController::MoveLeft()
{
	if(!moveLeft_)
	{
		characterMovementComponent_->AddMovementDirection(Vector3::LeftVector);
		moveLeft_ = true;
	}
}

void DefaultCharacterController::StopMovingLeft()
{
	if(moveLeft_)
	{
		characterMovementComponent_->AddMovementDirection(-Vector3::LeftVector);
		moveLeft_ = false;
	}
}

void DefaultCharacterController::MoveRight()
{
	if(!moveRight_)
	{
		characterMovementComponent_->AddMovementDirection(-Vector3::LeftVector);
		moveRight_ = true;
	}
}

void DefaultCharacterController::StopMovingRight()
{
	if(moveRight_)
	{
		characterMovementComponent_->AddMovementDirection(Vector3::LeftVector);
		moveRight_ = false;
	}
}

void DefaultCharacterController::StartRunning()
{
	characterMovementComponent_->StartRunning();
}

void DefaultCharacterController::StopRunning()
{
	characterMovementComponent_->StopRunning();
}

void DefaultCharacterController::Jump()
{
	if(characterMovementComponent_->CanJump())
	{
		characterMovementComponent_->Jump();
		character_->Jump();
	}
}

void DefaultCharacterController::Fire()
{
	character_->Fire();
}

void DefaultCharacterController::OnPause()
{
	Game* game = dynamic_cast<Game*>(engine->GetApplication());
	GameState* gameState = game->GetGameState();
	if (gameState->GetIsGamePaused())
	{
		gameState->ResumeGame();
	}
	else
	{
		gameState->PauseGame();
	}
}

void DefaultCharacterController::OnCursorMove(double x, double y)
{
	Vector2i windowSize = engine->GetWindowManager()->GetWindowSize();
	Vector2 windowCenter = windowSize * 0.5f;
	Vector2i currentCursorPosition = Vector2i(x, y);

	Vector2i difference = currentCursorPosition - windowCenter;
	if(GoknarMath::Abs(difference.x) + GoknarMath::Abs(difference.y) < 4)
	{
		difference = Vector2i::ZeroVector;
	}

	cursorDeltaMoveLastFrame_ = difference;

	engine->GetInputManager()->SetCursorPosition(windowCenter.x, windowCenter.y);
}
