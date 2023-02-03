#include "GameController.h"

// Engine includes
#include "Goknar/Engine.h"
#include "Goknar/Managers/InputManager.h"

// Game includes
#include "GameManager.h"
#include "GridManager.h"
#include "MovableObject.h"

GameController* GameController::instance_ = nullptr;

GameController::GameController() : Controller()
{
}

void GameController::SetupInputs()
{
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_PRESS, std::bind(&GameController::IncreaseCurrentBoneId, this));
}

void GameController::IncreaseCurrentBoneId()
{
}