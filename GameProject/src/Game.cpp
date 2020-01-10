#include <Goknar.h>

#include "Goknar/Scene.h"

#include "BoxGameObject.h"

class Game : public Application
{
public:
	Game();

	~Game() = default;

	void Run() override;

private:

	BoxGameObject* boxGameObject;
};

Game::Game() : Application()
{
	engine->SetApplication(this);

	//mainScene_->ReadSceneData("./Content/Scenes/SphereScene.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/MonkeyScene.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/ThreeSpheres.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/ThreeDifferentShapes.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/SphereWithNormals.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/ThreeDifferentShapesTransformations.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/2DScene.xml");
	//mainScene_->ReadSceneData("./Content/Scenes/GameProjectScene.xml");
	mainScene_->ReadSceneData("./Content/Scenes/AirPlane.xml");

	boxGameObject = new BoxGameObject();
}

void Game::Run()
{
}

Application *CreateApplication()
{
	return new Game();
}
