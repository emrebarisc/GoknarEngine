#include <Goknar.h>

#include "Airplane.h"
#include "CameraController.h"
#include "Goknar/Scene.h"

#include "Bat.h"
#include "Environment.h"

#include <chrono>

class Game : public Application
{
public:
	Game();
	~Game() = default;
	void Run() override;

private:
	Bat* batObject_;
	Environment* environment_;
};

Game::Game() : Application()
{
	engine->SetApplication(this);
	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	mainScene_->ReadSceneData("./Content/Scenes/ScaryMammal.xml");

	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_WARN("Scene is read in {} seconds.", elapsedTime);

	lastFrameTimePoint = currentTimePoint;
	batObject_ = new Bat();
	environment_ = new Environment();
}

void Game::Run()
{
}

Application *CreateApplication()
{
	return new Game();
}
