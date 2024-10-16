#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <vector>

#include "Core.h"

class DebugDrawer;

class TimeDependentObject;
class Application;
class CameraManager;
class Component;
class Controller;
class Editor;
class InputManager;
class ObjectBase;
class ObjectManager;
class Renderer;
class ResourceManager;
class Shader;
class WindowManager;

class DynamicMesh;
class StaticMesh;
class SkeletalMesh;

class PhysicsWorld;

// Global Engine variable
GOKNAR_API extern class Engine *engine;

class GOKNAR_API Engine
{
public:
	Engine();
	~Engine();

	void PreInit() const;
	void Init() const;
	void PostInit() const;

	void BeginGame();

	void PreInitObjects();
	void InitObjects();
	void PostInitObjects();
	void BeginGameObjects();

	void PreInitComponents();
	void InitComponents();
	void PostInitComponents();
	void BeginGameComponents();

	void Run();
	void Tick(float deltaTime);

	inline WindowManager* GetWindowManager() const
	{
		return windowManager_;
	}

	inline InputManager* GetInputManager() const
	{
		return inputManager_;
	}

	inline Renderer* GetRenderer() const
	{
		return renderer_;
	}

	void AddStaticMeshToRenderer(StaticMesh* staticMesh);
	void AddSkeletalMeshToRenderer(SkeletalMesh* skeletalMesh);
	void AddDynamicMeshToRenderer(DynamicMesh* dynamicMesh);

	inline Editor* GetEditor() const
	{
		return editor_;
	}

	inline CameraManager* GetCameraManager() const
	{
		return cameraManager_;
	}

	inline Application* GetApplication() const
	{
		return application_;
	}

	inline Controller* GetController()
	{
		return controller_;
	}

	inline void SetController(Controller* controller)
	{
		controller_ = controller;
	}

	inline ResourceManager* GetResourceManager() const
	{
		return resourceManager_;
	}

	inline PhysicsWorld* GetPhysicsWorld() const
	{
		return physicsWorld_;
	}
	
	void RegisterObject(ObjectBase *object);
	void RemoveObject(ObjectBase* object);
	void AddToTickableObjects(ObjectBase* object);
	void RemoveFromTickableObjects(ObjectBase* object);
	void RemoveFromObjectToBeInitialized(ObjectBase* object);
	void DestroyAllObjectsAndComponents();

	void RegisterComponent(Component* component);
	void RemoveComponent(Component* component);
	void AddToTickableComponents(Component* component);
	void RemoveFromTickableComponents(Component* component);
	void RemoveFromComponentsToBeInitialized(Component* component);

	void RegisterTimeDependentObject(TimeDependentObject* animatedMesh2D);

	void SetApplication(Application* application);

	void SetShaderEngineVariables(Shader* shader);

	void SetTimeScale(float timeScale)
	{
		timeScale_ = timeScale;
	}

	float GetTimeScale() const
	{
		return timeScale_;
	}

	float GetDeltaTime() const
	{
		return deltaTime_;
	}

	float GetElapsedTime() const
	{
		return elapsedTime_;
	}

	void DestroyObject(ObjectBase* object);
	void DestroyComponent(Component* component);

	void Exit();

private:
	DebugDrawer* debugDrawer_{ nullptr };

	InputManager* inputManager_;
	ResourceManager* resourceManager_;
	ObjectManager* objectManager_;
	Renderer* renderer_;
	WindowManager* windowManager_;
	Editor* editor_{ nullptr };
	CameraManager* cameraManager_;
	Controller* controller_{ nullptr };
	PhysicsWorld* physicsWorld_{ nullptr };

	Application* application_{ nullptr };

	std::vector<ObjectBase*> objectsToBeInitialized_;
	std::vector<ObjectBase*> registeredObjects_;
	std::vector<ObjectBase*> tickableObjects_;

	std::vector<Component*> componentsToBeInitialized_;
	std::vector<Component*> registeredComponents_;
	std::vector<Component*> tickableComponents_;

	std::vector<TimeDependentObject*> timeDependentObjects_;

	int objectsToBeInitializedSize_{ 0 };
	int componentsToBeInitializedSize_{ 0 };

	float timeScale_{ 1.f };
	float deltaTime_{ 0.f };
	float elapsedTime_{ 0.f };

	bool hasUninitializedObjects_{ false };
	bool hasUninitializedComponents_{ false };
};

#endif
