#pragma once

#include <Goknar/Application.h>

class GameController;
class UI;

class Game : public Application
{
public:
	Game();

	~Game() = default;

	void Run() override;

	UI* GetUI()
	{
		return ui_;
	}

private:
	GameController* gameController_;
	UI* ui_;
};