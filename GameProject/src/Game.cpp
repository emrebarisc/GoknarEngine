#include <Goknar.h>



class Game : public Application
{
public:
	Game();

	~Game() = default;
};

Game::Game() : Application()
{
	//mainScene_->ReadSceneData("../GameProject/Content/Scenes/SphereScene.xml");
	//mainScene_->ReadSceneData("../GameProject/Content/Scenes/MonkeyScene.xml");
	//mainScene_->ReadSceneData("../GameProject/Content/Scenes/ThreeSpheres.xml");
	mainScene_->ReadSceneData("../GameProject/Content/Scenes/ThreeDifferentShapes.xml");
	//mainScene_->ReadSceneData("../GameProject/Content/Scenes/SphereWithNormals.xml");
}

Application *CreateApplication()
{
	return new Game();
}
