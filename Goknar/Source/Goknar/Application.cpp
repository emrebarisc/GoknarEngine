#include "pch.h"

#include "Application.h"
#include "Log.h"
#include "Scene.h"

Application::Application() :
	applicationType_(AppType::Application3D)
{
	mainScene_ = new Scene();
}

Application::~Application()
{
	delete mainScene_;
}

void Application::PreInit()
{
	mainScene_->PreInit();
}

void Application::Init()
{
	mainScene_->Init();
}

void Application::PostInit()
{
	mainScene_->PostInit();
}

void Application::Run()
{

}
