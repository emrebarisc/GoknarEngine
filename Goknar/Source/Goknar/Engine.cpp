#include "pch.h"

// Goknar Libraries
#include "Engine.h"

#include "Application.h"
#include "Managers/CameraManager.h"
#include "Controller.h"
#include "Editor/ImGuiEditor/ImGuiEditor.h"
#include "Managers/InputManager.h"
#include "Log.h"
#include "ObjectBase.h"
#include "Managers/ObjectIDManager.h"
#include "Managers/ObjectManager.h"
#include "Managers/ResourceManager.h"
#include "Renderer/Renderer.h"
#include "Scene.h"
#include "Renderer/Shader.h"
#include "Renderer/ShaderBuilder.h"
#include "Lights/ShadowManager/ShadowManager.h"
#include "TimeDependentObject.h"
#include "Managers/WindowManager.h"

#include "Goknar/Lights/Light.h"
#include "Goknar/Lights/DirectionalLight.h"
#include "Goknar/Lights/PointLight.h"
#include "Goknar/Lights/SpotLight.h"

// OpenGL Libraries
#include "GLFW/glfw3.h"

#define GOKNAR_EDITOR true

GOKNAR_API Engine *engine;

Engine::Engine() :
	deltaTime_(0.f),
	elapsedTime_(0.f),
	application_(nullptr),
	editor_(nullptr),
	controller_(nullptr)
{
	engine = this;

	Log::Init();

	windowManager_ = new WindowManager();

	inputManager_ = new InputManager();
	resourceManager_ = new ResourceManager();
	objectManager_ = new ObjectManager();
	renderer_ = new Renderer();
#if GOKNAR_EDITOR
	editor_ = new ImGuiEditor();
#endif
	cameraManager_ = new CameraManager();

	// TODO
	//application_ = CreateApplication();
}

Engine::~Engine()
{
	delete renderer_;
	delete cameraManager_;
#if GOKNAR_EDITOR
	delete editor_;
#endif
	delete objectManager_;
	delete resourceManager_;
	delete inputManager_;
	delete application_;

	// Delete singletons
	delete ObjectIDManager::GetInstance();
	delete ShaderBuilder::GetInstance();

	delete controller_;

	delete windowManager_;
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

	ShaderBuilder::GetInstance()->Init();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Shader Builder Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;

	resourceManager_->Init();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Resource Manager Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;

	// TODO: CHECK IF NECESSARY!
	objectManager_->Init();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Object Manager Initialization: {} s.", elapsedTime);
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

#if GOKNAR_EDITOR
	editor_->Init();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Editor Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;
#endif

	renderer_->Init();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Renderer Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;
}

void Engine::Run()
{
	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	while (!windowManager_->GetWindowShouldBeClosed())
	{
		if (1.f < deltaTime_)
		{
			deltaTime_ = 1.f / 60.f;
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

		//renderer_->GetShadowManager()->RenderShadowMaps();
		renderer_->Render();

#if GOKNAR_EDITOR
		editor_->Tick(deltaTime_);
#endif
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

	for (TimeDependentObject* object : timeDependentObjects_)
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
	size_t registeredObjectsSize = registeredObjects_.size();

	for (size_t registeredObjectIndex = 0; registeredObjectIndex < registeredObjectsSize; registeredObjectIndex++)
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

void Engine::RegisterTimeDependentObject(TimeDependentObject* animatedObject)
{
	timeDependentObjects_.push_back(animatedObject);
}

void Engine::RemoveFromTickableObjects(ObjectBase* object)
{
	size_t tickableObjectsSize = tickableObjects_.size();

	for (size_t tickableObjectIndex = 0; tickableObjectIndex < tickableObjectsSize; tickableObjectIndex++)
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

void Engine::AddStaticMeshToRenderer(StaticMesh* staticMesh)
{
	renderer_->AddStaticMeshToRenderer(staticMesh);
}

void Engine::AddSkeletalMeshToRenderer(SkeletalMesh* skeletalMesh)
{
	renderer_->AddSkeletalMeshToRenderer(skeletalMesh);
}

void Engine::AddDynamicMeshToRenderer(DynamicMesh* dynamicMesh)
{
	renderer_->AddDynamicMeshToRenderer(dynamicMesh);
}

void Engine::SetApplication(Application* application)
{
	application_ = application;
}

void Engine::SetShaderEngineVariables(Shader* shader)
{
	shader->SetFloat(SHADER_VARIABLE_NAMES::TIMING::DELTA_TIME, deltaTime_);
	shader->SetFloat(SHADER_VARIABLE_NAMES::TIMING::ELAPSED_TIME, elapsedTime_);

	renderer_->SetLightUniforms(shader);
}