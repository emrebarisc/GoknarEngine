#pragma once

#include "Core.h"

namespace Goknar
{
	extern class Application* CreateApplication();

	class GOKNAR_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};
}
