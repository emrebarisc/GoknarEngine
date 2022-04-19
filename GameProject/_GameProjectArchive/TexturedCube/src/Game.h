#pragma once

#include <Goknar/Application.h>

class GameController;
class UI;

class Game : public Application
{
public:
	Game();

	~Game();

	void Run() override;
private:
};