#include "Game.h"

// Engine includes
#include "Goknar.h"
#include "Goknar/Scene.h"

// Game includes
#include "GameController.h"
#include "MovableObjectFactory.h"
#include "UI.h"

#include <chrono>

Game::Game() : Application()
{
	engine->SetApplication(this);
	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	mainScene_->ReadSceneData("./Content/Scenes/GGJ2020.xml");

	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_WARN("Scene is read in {} seconds.", elapsedTime);

	lastFrameTimePoint = currentTimePoint;

	gameController_ = GameController::GetInstance();
	MovableObjectFactory::GetInstance()->CreateMovableObject();

	ui_ = new UI();
}

void Game::Run()
{
}

Application* CreateApplication()
{
	return new Game();
}
