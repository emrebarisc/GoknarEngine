#include <Goknar.h>

#include "Goknar/Scene.h"

#include "Archer.h"
#include "GameController.h"

#include <chrono>

class Game : public Application
{
public:
	Game();

	~Game()
	{
	}

	void Run() override;

private:

	Archer* archer;
	GameController* gameController;
};

Game::Game() : Application()
{
	engine->SetApplication(this);
	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	mainScene_->ReadSceneData("Scenes/MainScene.xml");

	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_WARN("Scene is read in {} seconds.", elapsedTime);

	lastFrameTimePoint = currentTimePoint;

	archer = new Archer();
	gameController = new GameController();
	gameController->SetArcher(archer);
}

void Game::Run()
{
}

Application *CreateApplication()
{
	return new Game();
}
