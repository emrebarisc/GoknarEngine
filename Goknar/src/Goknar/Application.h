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

	void SetMainScene(Scene* scene)
	{
		mainScene_ = scene;
	}

	const Scene* GetMainScene() const
	{
		return mainScene_;
	}

protected:
	Scene* mainScene_;
};