#include "pch.h"

// Goknar Libraries
#include "Engine.h"

#include "Application.h"
#include "Controller.h"
#include "Managers/CameraManager.h"
#include "Editor/ImGuiEditor/ImGuiEditor.h"
#include "Managers/InputManager.h"
#include "Log.h"
#include "ObjectBase.h"
#include "Managers/ObjectManager.h"
#include "Renderer/Renderer.h"
#include "Scene.h"
#include "Renderer/Shader.h"
#include "Renderer/ShaderBuilder.h"
#include "Managers/WindowManager.h"

// OpenGL Libraries
#include "GLFW/glfw3.h"

GOKNAR_API Engine *engine;

Engine::Engine() : deltaTime_(0.f), elapsedTime_(0.f), application_(nullptr), editor_(nullptr), controller_(nullptr)
{
	engine = this;

	Log::Init();

	windowManager_ = new WindowManager();

	inputManager_ = new InputManager();
	objectManager_ = new ObjectManager();
	renderer_ = new Renderer();
	//editor_ = new ImGuiEditor();
	cameraManager_ = new CameraManager();

	// TODO
	//application_ = CreateApplication();
}

Engine::~Engine()
{
	delete cameraManager_;
	//delete editor_;
	delete renderer_;
	delete objectManager_;
	delete inputManager_;
	delete windowManager_;
	delete application_;
	
	glfwTerminate();
}

void Engine::Init() const
{
	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	windowManager_->Init();
	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Window Manager Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;


	inputManager_->Init();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Input Manager Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;

	// TODO: CHECK IF NECESSARY!
	objectManager_->Init();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Object Manager Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;


	ShaderBuilder::GetInstance()->Init();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Shader Builder Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;


	application_->Init();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Application Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;

	if (controller_)
	{
		controller_->SetupInputs();
	}

	renderer_->Init();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Renderer Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;


	//editor_->Init();
	//currentTimePoint = std::chrono::steady_clock::now();
	//elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	//GOKNAR_CORE_INFO("Editor Initialization: {} s.", elapsedTime);
	//lastFrameTimePoint = currentTimePoint;
}

void Engine::Run()
{
	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	while (!windowManager_->GetWindowShouldBeClosed())
	{
		if (1.f < deltaTime_)
		{
			deltaTime_ = 0.f;
			continue;
		}

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
		//editor_->Tick(deltaTime_);
		windowManager_->Update();

		currentTimePoint = std::chrono::steady_clock::now();
		deltaTime_ = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
		elapsedTime_ += deltaTime_;

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

void Engine::DestroyObject(ObjectBase* object)
{
	RemoveObject(object);
	if (object->GetTickable())
	{
		engine->RemoveFromTickableObjects(object);
	}
	delete object;
}

void Engine::RegisterObject(ObjectBase* object)
{
	registeredObjects_.push_back(object);
}

void Engine::RemoveObject(ObjectBase* object)
{
	int registeredObjectsSize = registeredObjects_.size();

	for (int registeredObjectIndex = 0; registeredObjectIndex < registeredObjectsSize; registeredObjectIndex++)
	{
		if (registeredObjects_[registeredObjectIndex] == object)
		{
			registeredObjects_.erase(registeredObjects_.begin() + registeredObjectIndex);
			return;
		}
	}
}

void Engine::AddToTickableObjects(ObjectBase* object)
{
	tickableObjects_.push_back(object);
}

void Engine::RemoveFromTickableObjects(ObjectBase* object)
{
	int tickableObjectsSize = tickableObjects_.size();

	for (int tickableObjectIndex = 0; tickableObjectIndex < tickableObjectsSize; tickableObjectIndex++)
	{
		if (tickableObjects_[tickableObjectIndex] == object)
		{
			tickableObjects_.erase(tickableObjects_.begin() + tickableObjectIndex);
			return;
		}
	}
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

void Engine::SetShaderEngineVariables(Shader* shader)
{
	shader->SetFloat(SHADER_VARIABLE_NAMES::TIMING::DELTA_TIME, deltaTime_);
	shader->SetFloat(SHADER_VARIABLE_NAMES::TIMING::ELAPSED_TIME, elapsedTime_);
}
