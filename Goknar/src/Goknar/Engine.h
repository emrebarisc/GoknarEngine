#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <vector>

#include "Core.h"

class TimeDependentObject;
class Application;
class CameraManager;
class Controller;
class Editor;
class InputManager;
class ObjectBase;
class ObjectManager;
class Renderer;
class Shader;
class WindowManager;

class DynamicMesh;
class StaticMesh;

// Global Engine variable
GOKNAR_API extern class Engine *engine;

class GOKNAR_API Engine
{
public:
	Engine();
	~Engine();

	void Init() const;
	void BeginGame();
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
	
	void RegisterObject(ObjectBase *object);
	void RemoveObject(ObjectBase* object);
	void AddToTickableObjects(ObjectBase* object);
	void RemoveFromTickableObjects(ObjectBase* object);
	void RegisterAnimatedObject(TimeDependentObject* animatedMesh2D);

	void SetApplication(Application* application);

	void SetShaderEngineVariables(Shader* shader);

	float GetDeltaTime() const
	{
		return deltaTime_;
	}

	float GetElapsedTime() const
	{
		return elapsedTime_;
	}

	void DestroyObject(ObjectBase* object);

	void Exit();

private:
	InputManager* inputManager_;
	ObjectManager* objectManager_;
	Renderer* renderer_;
	WindowManager* windowManager_;
	Editor* editor_;
	CameraManager* cameraManager_;
	Controller* controller_;

	Application *application_;
	
	std::vector<ObjectBase *> registeredObjects_;
	std::vector<ObjectBase *> tickableObjects_;
	std::vector<TimeDependentObject*> timeDependentObjects_;

	float deltaTime_;
	float elapsedTime_;
};

#endif
