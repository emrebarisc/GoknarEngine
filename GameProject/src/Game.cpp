#include <Goknar.h>

class Game : public Application
{
public:
	Game()
	{
		
	}

	~Game()
	{
		
	}
};

Application *CreateApplication()
{
	return new Game();
}