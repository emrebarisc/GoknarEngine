#pragma once
#include "Engine.h"

int main(int argc, char **argv)
{
	Engine *mainEngine = new Engine();
	mainEngine->SetApplication(CreateApplication());
	mainEngine->Init();
	mainEngine->BeginGame();
	mainEngine->Run();
	delete mainEngine;
	
	return 0;
}
