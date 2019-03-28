#include <Goknar.h>

class Game : public Goknar::Application
{
public:
	Game()
	{
		
	}

	~Game()
	{
		
	}
};

Goknar::Application *Goknar::CreateApplication()
{
	return new Game();
}