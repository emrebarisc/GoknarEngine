#pragma once

#include <Goknar/Application.h>

class Deceased;
class Hyena;

class Game : public Application
{
public:
	Game();

	~Game();

	void Run() override;

private:
	Deceased* deceased_;
	Hyena* hyena_;
};