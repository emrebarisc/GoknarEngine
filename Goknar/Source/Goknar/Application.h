#ifndef __APPLICATION_H__
#define __APPLICATION_H__

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

	void PreInit();
	void Init();
	void PostInit();

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

#endif