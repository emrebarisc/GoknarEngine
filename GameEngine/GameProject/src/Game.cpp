#include <GameEngine.h>

class Game : public GameEngine::Application
{
public:
	Game()
	{
		
	}

	~Game()
	{
		
	}
};

GameEngine::Application *GameEngine::CreateApplication()
{
	return new Game();
}