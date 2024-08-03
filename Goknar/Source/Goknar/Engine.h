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

class HUD;

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

	inline CameraManager* GetCameraManager() const
	{
		return cameraManager_;
	}

	inline Application* GetApplication() const
	{
		return application_;
	}

	inline ResourceManager* GetResourceManager() const
	{
		return resourceManager_;
	}

	inline PhysicsWorld* GetPhysicsWorld() const
	{
		return physicsWorld_;
	}

	inline void SetHUD(HUD* HUD)
	{
		HUD_ = HUD;
	}

	inline HUD* GetHUD() const
	{
		return HUD_;
	}
	
	void RegisterObject(ObjectBase *object);
	void AddToTickableObjects(ObjectBase* object);
	void RemoveFromTickableObjects(ObjectBase* object);
	void RemoveFromObjectToBeInitialized(ObjectBase* object);
	void DestroyAllObjectsAndComponents();
	
	template<class T = ObjectBase>
	std::vector<T*> GetObjectsOfType() const;

	void RegisterComponent(Component* component);
	void AddToTickableComponents(Component* component);
	void RemoveFromTickableComponents(Component* component);
	void RemoveFromComponentsToBeInitialized(Component* component);

	void RegisterTimeDependentObject(TimeDependentObject* timeDependentObject);
	void RemoveTimeDependentObject(TimeDependentObject* timeDependentObject);

	const std::vector<ObjectBase*>& GetRegisteredObjects() const
	{
		return registeredObjects_;
	}

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

	void AddObjectToDestroy(ObjectBase* object);
	void AddComponentToDestroy(Component* component);

	void Exit();

private:
	void ClearMemory();

	void DestroyAllPendingObjectAndComponents();

	void DestroyObject(ObjectBase* object);
	void RemoveObject(ObjectBase* object);

	void DestroyComponent(Component* component);
	void RemoveComponent(Component* component);

	void RegisterTimeDependentObjects();

	DebugDrawer* debugDrawer_{ nullptr };

	InputManager* inputManager_;
	ResourceManager* resourceManager_;
	ObjectManager* objectManager_;
	Renderer* renderer_;
	WindowManager* windowManager_;
	CameraManager* cameraManager_;
	PhysicsWorld* physicsWorld_{ nullptr };
	HUD* HUD_{ nullptr };

	Application* application_{ nullptr };

	std::vector<ObjectBase*> objectsToBeInitialized_;
	std::vector<ObjectBase*> registeredObjects_;
	std::vector<ObjectBase*> tickableObjects_;

	std::vector<Component*> componentsToBeInitialized_;
	std::vector<Component*> registeredComponents_;
	std::vector<Component*> tickableComponents_;

	std::vector<ObjectBase*> objectsPendingDestroy_;
	std::vector<Component*> componentsPendingDestroy_;

	std::vector<TimeDependentObject*> timeDependentObjects_;
	std::vector<TimeDependentObject*> timeDependentObjectsToRegisterForNextFrame_;

	int objectsToBeInitializedSize_{ 0 };
	int componentsToBeInitializedSize_{ 0 };
	int timeDependentObjectsToBeRegisteredSize_{ 0 };

	float timeScale_{ 1.f };
	float deltaTime_{ 0.f };
	float elapsedTime_{ 0.f };

	bool hasUninitializedObjects_{ false };
	bool hasUninitializedComponents_{ false };
	bool hasObjectsOrComponentsPendingDestroy_{ false };
};

template<class T>
std::vector<T*> Engine::GetObjectsOfType() const
{
	std::vector<T*> result;

	std::vector<ObjectBase*>::const_iterator registeredObjectsIterator = registeredObjects_.cbegin();

	while (registeredObjectsIterator != registeredObjects_.cend())
	{
		T* requiredTypeObject = dynamic_cast<T*>(*registeredObjectsIterator);
		if (requiredTypeObject && !requiredTypeObject->isPendingDestroy_)
		{
			result.push_back(requiredTypeObject);
		}

		++registeredObjectsIterator;
	}

	return result;
}

#endif
