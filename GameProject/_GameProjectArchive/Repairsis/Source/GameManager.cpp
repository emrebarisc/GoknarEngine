#include "GameManager.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"

#include "Game.h"
#include "GridManager.h"
#include "MovableObjectFactory.h"
#include "UI.h"

#include <iostream>

GameManager* GameManager::instance_ = nullptr;

void GameManager::Restart()
{
	GridManager* gridManagerInstance = GridManager::GetInstance();
	gridManagerInstance->ClearGrid();

	Game* game = dynamic_cast<Game*>(engine->GetApplication());
	if (game)
	{
		game->GetUI()->HideGameOverUI();
	}

	isGameOver_ = false;
	score_ = 0;

	MovableObjectFactory::GetInstance()->CreateMovableObject();

}

void GameManager::GameOver()
{
	isGameOver_ = true;

	Game* game = dynamic_cast<Game*>(engine->GetApplication());
	if (game)
	{
		game->GetUI()->ShowGameOverUI();
		std::cout << "Press Space To Restart." << std::endl;
	}
}

void GameManager::IncreaseScore(float amount)
{
	score_ += amount;
	std::cout << "Score: " << score_ << std::endl;
}
