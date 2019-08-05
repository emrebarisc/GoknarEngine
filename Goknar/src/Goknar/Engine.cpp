#include "pch.h"

#include "Engine.h"

#include "GLFW/glfw3.h"

#include "Application.h"
#include "Editor/ImGuiEditor/ImGuiEditor.h"
#include "Log.h"
#include "Managers/CameraManager.h"
#include "Managers/InputManager.h"
#include "Managers/ObjectManager.h"
#include "Managers/Renderer.h"
#include "Managers/WindowManager.h"
#include "ObjectBase.h"
#include "Scene.h"

Engine *engine;

Engine::Engine()
{
	engine = this;

	Log::Init();

	windowManager_ = new WindowManager();
	windowManager_->SetWindowWidth(1024);
	windowManager_->SetWindowHeight(768);
	windowManager_->SetContextVersion(4, 3);
	windowManager_->SetWindowTitle("Goknar Engine");

	inputManager_ = new InputManager();
	objectManager_ = new ObjectManager();
	renderer_ = new Renderer();
	editor_ = new ImGuiEditor();
	cameraManager_ = new CameraManager();
	
	// TODO
	//application_ = CreateApplication();
}

Engine::~Engine()
{
	delete cameraManager_;
	delete editor_;
	delete renderer_;
	delete objectManager_;
	delete inputManager_;
	delete windowManager_;
	delete application_;
	
	glfwTerminate();
}

void Engine::Init() const
{
	windowManager_->Init();
	inputManager_->Init();
	objectManager_->Init();
	renderer_->Init();
	Scene::mainScene->Init();

	editor_->Init();
}

void Engine::Run()
{
	while (!windowManager_->GetWindowShouldBeClosed())
	{
		application_->Run();

		// TODO Temp Tick Call
		Tick(0.016f);

		// TODO Renderer
		renderer_->Render();
		glfwPollEvents();
	}
}

void Engine::BeginGame()
{
	for (ObjectBase* object : registeredObjects_)
	{
		object->BeginGame();
	}
}

void Engine::Tick(float deltaTime)
{
	for (ObjectBase* object : tickableObjects_)
	{
		object->Tick(deltaTime);
	}

	renderer_->Render();
	editor_->Tick(deltaTime);
	windowManager_->Update();
}

void Engine::RegisterObject(ObjectBase* object)
{
	registeredObjects_.push_back(object);
}

void Engine::AddToTickableObjects(ObjectBase* object)
{
	tickableObjects_.push_back(object);
}

void Engine::Exit()
{
	windowManager_->CloseWindow();
}

void Engine::SetApplication(Application* application)
{
	application_ = application;
}
