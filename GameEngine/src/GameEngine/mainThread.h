#pragma once
#include "Application.h"

#include <iostream>

#ifdef GAMEENGINE_PLATFORM_WINDOWS

extern GameEngine::Application* GameEngine::CreateApplication();

int main(int argc, char **argv)
{
	GameEngine::Log::Init();
	ENGINE_CORE_WARN("Initialized Game Engine Log.");
	ENGINE_INFO("Initialized Application Log.");

	GameEngine::Application *application = GameEngine::CreateApplication();
	application->Run();
	delete application;

	return 0;
}

#endif