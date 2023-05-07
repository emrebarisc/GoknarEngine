#include <Goknar.h>

#include "Goknar/Scene.h"

#include "Goknar/Camera.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Managers/WindowManager.h"

#include "PhysicsObjectManager.h"
#include "RigidBodyManager.h"

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
	PhysicsObjectManager* physicsObjectManager_;
	RigidBodyManager* rigidBodyManager_;
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

	//physicsObjectManager_ = new PhysicsObjectManager();
	rigidBodyManager_ = new RigidBodyManager();

	engine->GetWindowManager()->SetIsInFullscreen(false);
	//engine->SetTimeScale(0.1f);
}

void Game::Run()
{
}

Application *CreateApplication()
{
	return new Game();
}
