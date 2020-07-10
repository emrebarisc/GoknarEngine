#include "Game.h"

// Engine includes
#include "Goknar.h"
#include "Goknar/Scene.h"

#include <chrono>

Game::Game() : Application()
{
	engine->SetApplication(this);
	SetAppType(AppType::Application3D);

	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	mainScene_->ReadSceneData("./Content/Scenes/SeaShader.xml");

	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_WARN("Scene is read in {} seconds.", elapsedTime);
}

Game::~Game()
{
}

void Game::Run()
{
}

Application* CreateApplication()
{
	return new Game();
}
