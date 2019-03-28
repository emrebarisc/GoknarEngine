#pragma once
#include "Scene.h"

int main(int argc, char **argv)
{
	Scene *scene = new Scene();

	Engine *mainEngine = new Engine();
	mainEngine->SetApplication(CreateApplication());
	mainEngine->Init();
	mainEngine->BeginGame();
	mainEngine->Run();
	delete mainEngine;

	delete scene;
	
	return 0;
}
