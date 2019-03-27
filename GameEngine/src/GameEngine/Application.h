#pragma once

#include "Core.h"


namespace GameEngine
{
	extern class Application* CreateApplication();

	class GAMEENGINE_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};
}
