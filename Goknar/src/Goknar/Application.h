#pragma once

#include "Core.h"

class Scene;

extern class Application* CreateApplication();

enum class AppType : unsigned char
{
	Application2D = 0x0,
	Application3D
};

class GOKNAR_API Application
{
public:
	Application();
	virtual ~Application();

	void Init();

	virtual void Run();

	void SetMainScene(Scene* scene)
	{
		mainScene_ = scene;
	}

	Scene* GetMainScene() const
	{
		return mainScene_;
	}

	AppType GetAppType() const
	{
		return applicationType_;
	}

	void SetAppType(AppType applicationType)
	{
		applicationType_ = applicationType;
	}

protected:
	Scene* mainScene_;

	AppType applicationType_;
};