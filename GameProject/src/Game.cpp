#include "Game.h"

#include <Goknar.h>

#include "CameraController.h"
#include "Goknar/Scene.h"

#include <chrono>

Game::Game() : Application()
{
	engine->SetApplication(this);
	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	mainScene_->ReadSceneData("./Content/RenderTestScene.xml");

	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_WARN("Scene is read in {} seconds.", elapsedTime);

	lastFrameTimePoint = currentTimePoint;
	cameraController_ = new CameraController();
}

Game::~Game()
{
}

void Game::Run()
{
}

Application *CreateApplication()
{
	return new Game();
}
