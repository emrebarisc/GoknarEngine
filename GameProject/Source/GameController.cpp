#include "GameController.h"

// Engine includes
#include "Goknar/Engine.h"
#include "Goknar/Managers/InputManager.h"

#include "Archer.h"
#include "Components/ArcherMovementComponent.h"

#define MAX_BONE_INDEX 37
#define MAX_ANIMATION_INDEX 19

GameController::GameController() : 
	Controller(),
	archer(nullptr)
{
}

void GameController::SetupInputs()
{
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_PRESS, std::bind(&GameController::MoveForward, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_RELEASE, std::bind(&GameController::StopMovingForward, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_PRESS, std::bind(&GameController::MoveBackward, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_RELEASE, std::bind(&GameController::StopMovingBackward, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_PRESS, std::bind(&GameController::MoveLeft, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_RELEASE, std::bind(&GameController::StopMovingLeft, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_PRESS, std::bind(&GameController::MoveRight, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_RELEASE, std::bind(&GameController::StopMovingRight, this));
}

void GameController::MoveForward()
{
}

void GameController::StopMovingForward()
{
}

void GameController::MoveBackward()
{
}

void GameController::StopMovingBackward()
{
}

void GameController::MoveLeft()
{
}

void GameController::StopMovingLeft()
{
}

void GameController::MoveRight()
{
}

void GameController::StopMovingRight()
{
}