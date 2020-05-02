#include "pch.h"

#include "Application.h"
#include "Log.h"
#include "Scene.h"

Application::Application()
{
	mainScene_ = new Scene();
}

Application::~Application()
{
	delete mainScene_;
}

void Application::Init()
{
	mainScene_->Init();
}

void Application::Run()
{

}
