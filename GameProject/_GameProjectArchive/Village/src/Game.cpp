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
	//mainScene_->ReadSceneData("./Content/Scenes/SphereScene.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/MonkeyScene.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/ThreeSpheres.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/ThreeDifferentShapes.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/SphereWithNormals.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/ThreeDifferentShapesTransformations.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/2DScene.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/AirPlane.xml");
	mainScene_->ReadSceneData("./Content/Scenes/Village.xml");

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
