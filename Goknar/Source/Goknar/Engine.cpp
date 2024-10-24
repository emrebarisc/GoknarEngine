#include "pch.h"

// Goknar Libraries
#include "Engine.h"

#include "Application.h"
#include "Controller.h"
#include "Log.h"
#include "ObjectBase.h"
#include "Scene.h"
#include "TimeDependentObject.h"
#include "Components/Component.h"
#include "Debug/DebugDrawer.h"
#include "Factories/DynamicObjectFactory.h"
#include "Managers/CameraManager.h"
#include "Managers/InputManager.h"
#include "Managers/ObjectIDManager.h"
#include "Managers/ObjectManager.h"
#include "Managers/ResourceManager.h"
#include "Managers/WindowManager.h"
#include "Physics/PhysicsWorld.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"
#include "Renderer/ShaderBuilder.h"
#include "Renderer/ShaderBuilderNew.h"
#include "UI/HUD.h"

#include "Lights/DirectionalLight.h"
#include "Lights/Light.h"
#include "Lights/PointLight.h"
#include "Lights/SpotLight.h"
#include "Lights/LightManager/LightManager.h"

// OpenGL Libraries
#include "GLFW/glfw3.h"

#define GOKNAR_EDITOR false

GOKNAR_API Engine* engine;

Engine::Engine()
{
	engine = this;

	Log::Init();

	windowManager_ = new WindowManager();

	inputManager_ = new InputManager();
	resourceManager_ = new ResourceManager();
	objectManager_ = new ObjectManager();
	renderer_ = new Renderer();
	physicsWorld_ = new PhysicsWorld();
	cameraManager_ = new CameraManager();

	debugDrawer_ = new DebugDrawer();

	// TODO
	//application_ = CreateApplication();
}

Engine::~Engine()
{
	delete physicsWorld_;
	physicsWorld_ = nullptr;

	ClearMemory();

	delete application_;
	application_ = nullptr;

	delete renderer_;
	renderer_ = nullptr;

	delete cameraManager_;
	cameraManager_ = nullptr;

	delete debugDrawer_;
	debugDrawer_ = nullptr;

	delete objectManager_;
	objectManager_ = nullptr;

	delete resourceManager_;
	resourceManager_ = nullptr;

	delete inputManager_;
	inputManager_ = nullptr;

	// Delete singletons
	delete ObjectIDManager::GetInstance();
	delete ShaderBuilder::GetInstance();
	delete ShaderBuilderNew::GetInstance();
	delete DynamicObjectFactory::GetInstance();

	delete windowManager_;
	windowManager_ = nullptr;
}

void Engine::PreInit() const
{
	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	windowManager_->PreInit();
	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Window Manager Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;

	inputManager_->PreInit();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Input Manager Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;

	application_->PreInit();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Application Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;

	ShaderBuilder::GetInstance()->Init();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Shader Builder Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;

	resourceManager_->PreInit();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Resource Manager Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;

	cameraManager_->PreInit();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Camera Manager Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;

	// TODO: CHECK IF NECESSARY!
	objectManager_->Init();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Object Manager Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;

	physicsWorld_->PreInit();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Physics Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;

	renderer_->PreInit();
	currentTimePoint = std::chrono::steady_clock::now();
	elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();
	GOKNAR_CORE_INFO("Renderer Initialization: {} s.", elapsedTime);
	lastFrameTimePoint = currentTimePoint;
}

void Engine::Init() const
{
	windowManager_->Init();

	inputManager_->Init();

	application_->Init();

	resourceManager_->Init();

	cameraManager_->Init();

	physicsWorld_->Init();

	renderer_->Init();
}

void Engine::PostInit() const
{
	windowManager_->PostInit();

	inputManager_->PostInit();

	application_->PostInit();

	resourceManager_->PostInit();

	cameraManager_->PostInit();

	physicsWorld_->PostInit();

	renderer_->PostInit();
}

void Engine::Run()
{
	std::chrono::steady_clock::time_point lastFrameTimePoint = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point currentTimePoint = std::chrono::steady_clock::now();
	while (!windowManager_->GetWindowShouldBeClosed())
	{
		// Initialize dynamically created object and components /////

		if (hasUninitializedComponents_)
		{
			PreInitComponents();
		}

		if (hasUninitializedObjects_)
		{
			PreInitObjects();
		}

		if (hasUninitializedComponents_)
		{
			InitComponents();
		}

		if (hasUninitializedObjects_)
		{
			InitObjects();
		}

		if (hasUninitializedComponents_)
		{
			PostInitComponents();
		}

		if (hasUninitializedObjects_)
		{
			PostInitObjects();
		}

		if (hasUninitializedComponents_)
		{
			BeginGameComponents();
		}

		if (hasUninitializedObjects_)
		{
			BeginGameObjects();
		}

		////////////////////////////////////////////////////////////////////////////

		if (0.25f < deltaTime_)
		{
			deltaTime_ = 0.25f;
		}

		{
			static int frameCount = 0;
			static float oneSecondCounter = 0.f;
			oneSecondCounter += deltaTime_;
			++frameCount;
			if (1.f < oneSecondCounter)
			{
				windowManager_->SetWindowTitle((std::string("FPS: ") + std::to_string(frameCount)).c_str());
				oneSecondCounter -= 1.f;
				frameCount = 0;
			}
		}

		const float unscaledDeltaTime = deltaTime_;
		deltaTime_ *= timeScale_;

		elapsedTime_ += deltaTime_;

		physicsWorld_->PhysicsTick(deltaTime_);

		application_->Run();
		Tick(deltaTime_);

		renderer_->RenderCurrentFrame();

		if (HUD_)
		{
			HUD_->Tick(unscaledDeltaTime);
		}

		windowManager_->Update();

		if (0 < timeDependentObjectsToBeRegisteredSize_)
		{
			RegisterTimeDependentObjects();
		}

		cameraManager_->HandleNewlyAddedCameras();

		if (hasObjectsOrComponentsPendingDestroy_)
		{
			DestroyAllPendingObjectAndComponents();
		}

		currentTimePoint = std::chrono::steady_clock::now();
		deltaTime_ = std::chrono::duration_cast<std::chrono::duration<float>>(currentTimePoint - lastFrameTimePoint).count();

		lastFrameTimePoint = currentTimePoint;
	}
}

void Engine::BeginGame()
{
	PreInitComponents();
	PreInitObjects();

	InitComponents();
	InitObjects();

	PostInitComponents();
	PostInitObjects();

	BeginGameComponents();
	BeginGameObjects();
}

void Engine::PreInitObjects()
{
	// Looping over vector via member size optimizes size() call and
	// Protects objectsToBeInitialized_ modification in mid air
	// For example creating new object on BeginGame() function
	for (int objectIndex = 0; objectIndex < objectsToBeInitializedSize_; ++objectIndex)
	{
		objectsToBeInitialized_[objectIndex]->PreInit();
	}

	GOKNAR_CORE_ASSERT(objectsToBeInitialized_.size() == objectsToBeInitializedSize_, "CANNOT ADD OBJECTS BEFORE INITIALIZATION!");
}

void Engine::InitObjects()
{
	// Looping over vector via member size optimizes size() call and
	// Protects objectsToBeInitialized_ modification in mid air
	// For example creating new object on BeginGame() function
	for (int objectIndex = 0; objectIndex < objectsToBeInitializedSize_; ++objectIndex)
	{
		objectsToBeInitialized_[objectIndex]->Init();
	}

	GOKNAR_CORE_ASSERT(objectsToBeInitialized_.size() == objectsToBeInitializedSize_, "CANNOT ADD OBJECTS BEFORE INITIALIZATION!");
}

void Engine::PostInitObjects()
{
	// Looping over vector via member size optimizes size() call and
	// Protects objectsToBeInitialized_ modification in mid air
	// For example creating new object on BeginGame() function
	for (int objectIndex = 0; objectIndex < objectsToBeInitializedSize_; ++objectIndex)
	{
		objectsToBeInitialized_[objectIndex]->PostInit();
	}

	GOKNAR_CORE_ASSERT(objectsToBeInitialized_.size() == objectsToBeInitializedSize_, "CANNOT ADD OBJECTS BEFORE INITIALIZATION!");
}

void Engine::BeginGameObjects()
{
	std::vector<ObjectBase*> objectsToBeInitialized;
	std::swap(objectsToBeInitialized, objectsToBeInitialized_);

	int objectsToBeInitializedSizeBeforeBeginGame = objectsToBeInitializedSize_;

	decltype(objectsToBeInitialized_.begin()) objectsToBeInitializedIterator = objectsToBeInitialized_.begin();

	for (int objectToBeInitializedIndex = 0; objectToBeInitializedIndex < objectsToBeInitializedSizeBeforeBeginGame; ++objectToBeInitializedIndex)
	{
		objectsToBeInitialized[objectToBeInitializedIndex]->BeginGame();
	}

	objectsToBeInitializedSize_ -= objectsToBeInitializedSizeBeforeBeginGame;
	hasUninitializedObjects_ = objectsToBeInitializedSize_ != 0;
}

void Engine::PreInitComponents()
{
	// Looping over vector via member size optimizes size() call and
	// Protects componentsToBeInitialized_ modification in mid air
	// For example creating new components on BeginGame() function
	for (int componentIndex = 0; componentIndex < componentsToBeInitializedSize_; ++componentIndex)
	{
		componentsToBeInitialized_[componentIndex]->PreInit();
	}

	GOKNAR_CORE_ASSERT(componentsToBeInitialized_.size() == componentsToBeInitializedSize_, "CANNOT ADD COMPONENTS BEFORE INITIALIZATION!");
}

void Engine::InitComponents()
{
	for (int componentIndex = 0; componentIndex < componentsToBeInitializedSize_; ++componentIndex)
	{
		componentsToBeInitialized_[componentIndex]->Init();
	}

	GOKNAR_CORE_ASSERT(componentsToBeInitialized_.size() == componentsToBeInitializedSize_, "CANNOT ADD COMPONENTS BEFORE INITIALIZATION!");
}

void Engine::PostInitComponents()
{
	for (int componentIndex = 0; componentIndex < componentsToBeInitializedSize_; ++componentIndex)
	{
		componentsToBeInitialized_[componentIndex]->PostInit();
	}

	GOKNAR_CORE_ASSERT(componentsToBeInitialized_.size() == componentsToBeInitializedSize_, "CANNOT ADD COMPONENTS BEFORE INITIALIZATION!");
}

void Engine::BeginGameComponents()
{
	std::vector<Component*> componentsToBeInitialized;
	std::swap(componentsToBeInitialized, componentsToBeInitialized_);

	int componentsToBeInitializedSizeBeforeBeginGame = componentsToBeInitializedSize_;

	decltype(componentsToBeInitialized_.begin()) componentsToBeInitializedIterator = componentsToBeInitialized_.begin();

	for (int objectToBeInitializedIndex = 0; objectToBeInitializedIndex < componentsToBeInitializedSizeBeforeBeginGame; ++objectToBeInitializedIndex)
	{
		componentsToBeInitialized[objectToBeInitializedIndex]->BeginGame();
	}

	componentsToBeInitializedSize_ -= componentsToBeInitializedSizeBeforeBeginGame;
	hasUninitializedComponents_ = componentsToBeInitializedSize_ != 0;
}

void Engine::Tick(float deltaTime)
{
	for (ObjectBase* object : tickableObjects_)
	{
		if (object->GetIsInitialized() && object->GetIsActive() && object->GetIsTickEnabled())
		{
			object->Tick(deltaTime);
		}
	}

	for (Component* component : tickableComponents_)
	{
		if (component->GetIsInitialized() && component->GetIsActive() && component->GetIsTickEnabled())
		{
			component->TickComponent(deltaTime);
		}
	}

	for (TimeDependentObject* object : timeDependentObjects_)
	{
		if (object->GetIsActive())
		{
			object->Tick(deltaTime);
		}
	}
}

void Engine::ClearMemory()
{
	std::vector<Component*>::iterator registeredComponentsIterator = registeredComponents_.begin();
	for (; registeredComponentsIterator != registeredComponents_.end(); ++registeredComponentsIterator)
	{
		Component* component = *registeredComponentsIterator;
		component->Destroy();
		component->DestroyInner();
	}

	std::vector<ObjectBase*>::iterator registeredObjectsIterator = registeredObjects_.begin();
	for (; registeredObjectsIterator != registeredObjects_.end(); ++registeredObjectsIterator)
	{
		ObjectBase* object = *registeredObjectsIterator;
		object->Destroy();
		object->DestroyInner();
	}

	registeredComponentsIterator = registeredComponents_.begin();
	for (; registeredComponentsIterator != registeredComponents_.end(); ++registeredComponentsIterator)
	{
		delete *registeredComponentsIterator;
	}

	registeredObjectsIterator = registeredObjects_.begin();
	for (; registeredObjectsIterator != registeredObjects_.end(); ++registeredObjectsIterator)
	{
		delete *registeredObjectsIterator;
	}

	registeredComponents_.clear();
	tickableComponents_.clear();
	componentsToBeInitialized_.clear();

	registeredObjects_.clear();
	tickableObjects_.clear();
	objectsToBeInitialized_.clear();
}

void Engine::DestroyAllPendingObjectAndComponents()
{
	// Destroy objects and components using indexed for loop
	// calculating size at every iteration
	// in order to handle newly pending objects and components
	// added in other object or component's destroy call
	for (int objectIndex = 0; objectIndex < objectsPendingDestroy_.size(); ++objectIndex)
	{
		DestroyObject(objectsPendingDestroy_[objectIndex]);
	}

	for (int componentIndex = 0; componentIndex < componentsPendingDestroy_.size(); ++componentIndex)
	{
		DestroyComponent(componentsPendingDestroy_[componentIndex]);
	}

	objectsPendingDestroy_.clear();
	componentsPendingDestroy_.clear();
	hasObjectsOrComponentsPendingDestroy_ = false;
}

void Engine::DestroyComponent(Component* component)
{
	RemoveComponent(component);
	if (component->GetIsTickable())
	{
		RemoveFromTickableComponents(component);
	}
	RemoveFromComponentsToBeInitialized(component);

	component->DestroyInner();

	delete component;
}

void Engine::DestroyObject(ObjectBase* object)
{
	RemoveObject(object);
	if (object->GetIsTickable())
	{
		RemoveFromTickableObjects(object);
	}

	RemoveFromObjectToBeInitialized(object);

	object->DestroyInner();

	delete object;
}

void Engine::DestroyAllObjectsAndComponents()
{
	std::vector<Component*>::iterator registeredComponentsIterator = registeredComponents_.begin();
	for (; registeredComponentsIterator != registeredComponents_.end(); ++registeredComponentsIterator)
	{
		(*registeredComponentsIterator)->Destroy();
	}
	registeredComponents_.clear();
	tickableComponents_.clear();
	componentsToBeInitialized_.clear();

	std::vector<ObjectBase*>::iterator registeredObjectsIterator = registeredObjects_.begin();
	for (; registeredObjectsIterator != registeredObjects_.end(); ++registeredObjectsIterator)
	{
		(*registeredObjectsIterator)->Destroy();
	}
}

void Engine::AddToObjectsToBeInitialized(ObjectBase* object)
{
	hasUninitializedObjects_ = true;
	objectsToBeInitialized_.push_back(object);
	objectsToBeInitializedSize_++;
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

void Engine::RegisterTimeDependentObjects()
{
	if (timeDependentObjects_.empty())
	{
		timeDependentObjects_ = std::move(timeDependentObjectsToRegisterForNextFrame_);
	}
	else
	{
		timeDependentObjects_.reserve(timeDependentObjects_.size() + timeDependentObjectsToBeRegisteredSize_);

		std::move(
			std::begin(timeDependentObjectsToRegisterForNextFrame_),
			std::end(timeDependentObjectsToRegisterForNextFrame_),
			std::back_inserter(timeDependentObjects_));
	}

	timeDependentObjectsToRegisterForNextFrame_.clear();
	timeDependentObjectsToBeRegisteredSize_ = 0;
}

void Engine::RegisterTimeDependentObject(TimeDependentObject* timeDependentObject)
{
	timeDependentObjectsToRegisterForNextFrame_.push_back(timeDependentObject);
	++timeDependentObjectsToBeRegisteredSize_;
}

void Engine::RemoveTimeDependentObject(TimeDependentObject* timeDependentObject)
{
	size_t timeDependentObjectsSize = timeDependentObjects_.size();

	for (size_t timeDependentObjectIndex = 0; timeDependentObjectIndex < timeDependentObjectsSize; timeDependentObjectIndex++)
	{
		if (timeDependentObjects_[timeDependentObjectIndex] == timeDependentObject)
		{
			timeDependentObjects_.erase(timeDependentObjects_.begin() + timeDependentObjectIndex);
			return;
		}
	}
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

void Engine::RemoveFromObjectToBeInitialized(ObjectBase* object)
{
	std::vector<ObjectBase*>::iterator objectsToBeInitializedIterator = objectsToBeInitialized_.begin();
	for (; objectsToBeInitializedIterator != objectsToBeInitialized_.end(); ++objectsToBeInitializedIterator)
	{
		if (*objectsToBeInitializedIterator == object)
		{
			objectsToBeInitialized_.erase(objectsToBeInitializedIterator);
			--objectsToBeInitializedSize_;
			hasUninitializedObjects_ = objectsToBeInitializedSize_ != 0;
			return;
		}
	}
}

void Engine::RegisterComponent(Component* component)
{
	registeredComponents_.push_back(component);
	AddToComponentsToBeInitialized(component);
}

void Engine::AddObjectToDestroy(ObjectBase* object)
{
	hasObjectsOrComponentsPendingDestroy_ = true;
	objectsPendingDestroy_.emplace_back(object);
}

void Engine::AddComponentToDestroy(Component* component)
{
	hasObjectsOrComponentsPendingDestroy_ = true;
	componentsPendingDestroy_.emplace_back(component);
}

void Engine::RemoveComponent(Component* component)
{
	size_t registeredComponentsSize = registeredComponents_.size();

	for (size_t registeredComponentIndex = 0; registeredComponentIndex < registeredComponentsSize; registeredComponentIndex++)
	{
		if (registeredComponents_[registeredComponentIndex] == component)
		{
			registeredComponents_.erase(registeredComponents_.begin() + registeredComponentIndex);
			return;
		}
	}
}

void Engine::AddToTickableComponents(Component* component)
{
	tickableComponents_.push_back(component);
}

void Engine::RemoveFromTickableComponents(Component* component)
{
	size_t tickableComponentsSize = tickableComponents_.size();

	for (size_t tickableComponentIndex = 0; tickableComponentIndex < tickableComponentsSize; tickableComponentIndex++)
	{
		if (tickableComponents_[tickableComponentIndex] == component)
		{
			tickableComponents_.erase(tickableComponents_.begin() + tickableComponentIndex);
			return;
		}
	}
}

void Engine::RemoveFromComponentsToBeInitialized(Component* component)
{
	std::vector<Component*>::iterator componentsToBeInitializedIterator = componentsToBeInitialized_.begin();
	for (; componentsToBeInitializedIterator != componentsToBeInitialized_.end(); ++componentsToBeInitializedIterator)
	{
		if (*componentsToBeInitializedIterator == component)
		{
			componentsToBeInitialized_.erase(componentsToBeInitializedIterator);
			--componentsToBeInitializedSize_;
			hasUninitializedObjects_ = componentsToBeInitializedSize_ != 0;
			return;
		}
	}
}

void Engine::AddToComponentsToBeInitialized(Component* component)
{
	hasUninitializedComponents_ = true;
	componentsToBeInitialized_.push_back(component);
	componentsToBeInitializedSize_++;
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
}