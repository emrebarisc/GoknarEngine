#include "Goknar.h"
#include "Goknar/Scene.h"

#include <chrono>

class Game : public Application
{
public:
	Game();

	~Game() = default;

	void Run() override;

private:
};

Game::Game() : Application()
{
	engine->SetApplication(this);
	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	//mainScene_->ReadSceneData("Scenes/SphereScene.xml");
	//mainScene_->ReadSceneData("Scenes/MonkeyScene.xml");
	//mainScene_->ReadSceneData("Scenes/ThreeSpheres.xml");
	//mainScene_->ReadSceneData("Scenes/ThreeDifferentShapes.xml");
	//mainScene_->ReadSceneData("Scenes/SphereWithNormals.xml");
	//mainScene_->ReadSceneData("Scenes/ThreeDifferentShapesTransformations.xml");
	//mainScene_->ReadSceneData("Scenes/2DScene.xml");
	//mainScene_->ReadSceneData("Scenes/AirPlane.xml");
	mainScene_->ReadSceneData("Scenes/Village.xml");

	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_WARN("Scene is read in {} seconds.", elapsedTime);

	lastFrameTimePoint = currentTimePoint;
}

void Game::Run()
{
}

Application *CreateApplication()
{
	return new Game();
}
