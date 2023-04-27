#include <Goknar.h>

#include "Goknar/Scene.h"

#include "Archer.h"
#include "Objects/AxisObject.h"
#include "Goknar/Managers/WindowManager.h"

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
	AxisObject* axisObject_;
	Archer* archer_;
};

Game::Game() : Application()
{
	engine->SetApplication(this);
	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	mainScene_->ReadSceneData("Scenes/Scene.xml");

	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_WARN("Scene is read in {} seconds.", elapsedTime);

	lastFrameTimePoint = currentTimePoint;

	archer_ = new Archer();

	axisObject_ = new AxisObject();
	axisObject_->SetParent(archer_, false, false);

	engine->GetWindowManager()->SetIsInFullscreen(false);
}

void Game::Run()
{
}

Application *CreateApplication()
{
	return new Game();
}
