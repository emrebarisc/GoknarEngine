#pragma once
#include "Application.h"

#include <iostream>

#ifdef GAMEENGINE_PLATFORM_WINDOWS

extern GameEngine::Application* GameEngine::CreateApplication();

int main(int argc, char **argv)
{
	std::cout << "Game Engine Instantiated." << std::endl;

	GameEngine::Application *application = GameEngine::CreateApplication();
	application->Run();
	delete application;

	return 0;
}

#endif