#pragma once

#include <Goknar/Application.h>

class Bat;
class Environment;

class Game : public Application
{
public:
	Game();

	~Game();

	void Run() override;

private:
	Bat* bat_;
	Environment* environment_;
};