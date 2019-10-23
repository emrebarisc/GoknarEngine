#include <Goknar.h>

#include "Goknar/Lights/PointLight.h"
#include "Goknar/Scene.h"

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
	//mainScene_->ReadSceneData("./Content/Scenes/SphereScene.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/MonkeyScene.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/ThreeSpheres.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/ThreeDifferentShapes.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/SphereWithNormals.xml");
	mainScene_->ReadSceneData("./Content/Scenes/ThreeDifferentShapesTransformations.xml");
}

void Game::Run()
{
}

Application *CreateApplication()
{
	return new Game();
}
