#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <vector>

#include "Core.h"

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

// Global Engine variable
GOKNAR_API extern class Engine *engine;

class GOKNAR_API Engine
{
public:
	Engine();
	~Engine();

	void Init() const;
	void BeginGame();
	void InitObjects();
	void InitComponents();
	void Run();
	void Tick(float deltaTime);

	WindowManager* GetWindowManager() const
	{
		return windowManager_;
	}

	InputManager* GetInputManager() const
	{
		return inputManager_;
	}

	Renderer* GetRenderer() const
	{
		return renderer_;
	}

	void AddStaticMeshToRenderer(StaticMesh* staticMesh);
	void AddSkeletalMeshToRenderer(SkeletalMesh* skeletalMesh);
	void AddDynamicMeshToRenderer(DynamicMesh* dynamicMesh);

	Editor* GetEditor() const
	{
		return editor_;
	}

	CameraManager* GetCameraManager() const
	{
		return cameraManager_;
	}

	Application* GetApplication() const
	{
		return application_;
	}

	Controller* GetController()
	{
		return controller_;
	}

	void SetController(Controller* controller)
	{
		controller_ = controller;
	}

	ResourceManager* GetResourceManager() const
	{
		return resourceManager_;
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
	InputManager* inputManager_;
	ResourceManager* resourceManager_;
	ObjectManager* objectManager_;
	Renderer* renderer_;
	WindowManager* windowManager_;
	Editor* editor_{ nullptr };
	CameraManager* cameraManager_;
	Controller* controller_{ nullptr };

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
