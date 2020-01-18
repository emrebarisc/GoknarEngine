#include "pch.h"

// Goknar Libraries
#include "Engine.h"
#include "Application.h"
#include "Log.h"
#include "ObjectBase.h"
#include "Scene.h"
#include "Renderer/ShaderBuilder.h"
#include "Editor/ImGuiEditor/ImGuiEditor.h"
#include "Managers/CameraManager.h"
#include "Managers/InputManager.h"
#include "Managers/ObjectManager.h"
#include "Renderer/Renderer.h"
#include "Managers/WindowManager.h"

// OpenGL Libraries
#include "GLFW/glfw3.h"

// C++ Standard Libraries
#include <chrono>

GOKNAR_API Engine *engine;

Engine::Engine() : deltaTime_(0.f)
{
	engine = this;

	Log::Init();

	windowManager_ = new WindowManager();

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
	ShaderBuilder::GetInstance()->Init();
	application_->Init();
	renderer_->Init();
	editor_->Init();
}

void Engine::Run()
{
	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	while (!windowManager_->GetWindowShouldBeClosed())
	{
		{
			static float oneSecondCounter = 0.f;
			oneSecondCounter += deltaTime_;
			if (1.f < oneSecondCounter)
			{
				windowManager_->SetWindowTitle((std::string("FPS: ") + std::to_string(int(1 / deltaTime_))).c_str());
				oneSecondCounter -= 1.f;
			}
		}

		application_->Run();
		Tick(deltaTime_);

		renderer_->Render();
		editor_->Tick(deltaTime_);
		windowManager_->Update();

		currentTimePoint = std::chrono::steady_clock::now();
		deltaTime_ = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();

		lastFrameTimePoint = currentTimePoint;
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

void Engine::AddObjectToRenderer(Mesh* mesh)
{
	renderer_->AddMeshToRenderer(mesh);
}

void Engine::SetApplication(Application* application)
{
	application_ = application;
}
