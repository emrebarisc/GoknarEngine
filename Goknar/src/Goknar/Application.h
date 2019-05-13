#pragma once

#include "Core.h"

class Scene;

extern class Application* CreateApplication();

class GOKNAR_API Application
{
public:
	Application();
	virtual ~Application();

	void Init();

	void Run();

protected:
	Scene* mainScene_;
};