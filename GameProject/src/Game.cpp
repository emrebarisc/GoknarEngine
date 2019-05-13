#include <Goknar.h>

class Game : public Application
{
public:
	Game();

	~Game() = default;
};

Game::Game() : Application()
{

}

Application *CreateApplication()
{
	return new Game();
}
