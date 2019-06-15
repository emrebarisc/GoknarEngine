#include <Goknar.h>



class Game : public Application
{
public:
	Game();

	~Game() = default;
};

Game::Game() : Application()
{
	mainScene_->ReadSceneData("../GameProject/Content/Scenes/BunnyScene.xml");
}

Application *CreateApplication()
{
	return new Game();
}
