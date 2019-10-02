#include <Goknar.h>

#include "Goknar/Lights/PointLight.h"

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
	//mainScene_->ReadSceneData("../GameProject/Content/Scenes/SphereScene.xml");
	//mainScene_->ReadSceneData("../GameProject/Content/Scenes/MonkeyScene.xml");
	//mainScene_->ReadSceneData("../GameProject/Content/Scenes/ThreeSpheres.xml");
	//mainScene_->ReadSceneData("../GameProject/Content/Scenes/ThreeDifferentShapes.xml");
	//mainScene_->ReadSceneData("../GameProject/Content/Scenes/SphereWithNormals.xml");
	mainScene_->ReadSceneData("../GameProject/Content/Scenes/ThreeDifferentShapesTransformations.xml");
}

void Game::Run()
{
}

Application *CreateApplication()
{
	return new Game();
}
