#include "Application.h"
#include "Log.h"
#include "Engine.h"

namespace GameEngine
{
	Application::Application()
	{

	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		ENGINE_CORE_INFO("Game is running.");
	}
}
