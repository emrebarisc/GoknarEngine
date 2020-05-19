#pragma once

#include <Goknar/Application.h>

class Deceased;

class Game : public Application
{
public:
	Game();

	~Game();

	void Run() override;

private:
	Deceased* deceased_;
};