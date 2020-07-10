#include <Goknar.h>

#include "Goknar/Scene.h"

#include <chrono>

#include "RenderToTextureObject.h"

class Game : public Application
{
public:
	Game();

	~Game() = default;

	void Run() override;

private:
	RenderToTextureObject* renderToTextureObject;
};

Game::Game() : Application()
{
	engine->SetApplication(this);
	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	mainScene_->ReadSceneData("./Content/Scenes/ShadowMapScene.xml");

	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_WARN("Scene is read in {} seconds.", elapsedTime);

	lastFrameTimePoint = currentTimePoint;

	renderToTextureObject = new RenderToTextureObject();
}

void Game::Run()
{
}

Application *CreateApplication()
{
	return new Game();
}
