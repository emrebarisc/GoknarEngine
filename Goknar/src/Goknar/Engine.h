#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <vector>

#include "Application.h"

class CameraManager;
class Editor;
class InputManager;
class Mesh;
class ObjectBase;
class ObjectManager;
class Renderer;
class Shader;
class WindowManager;

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

	void AddObjectToRenderer(Mesh* mesh);

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

	void SetApplication(Application* application);

	void SetShaderEngineVariables(Shader* shader);

	void RegisterObject(ObjectBase *object);
	void AddToTickableObjects(ObjectBase *object);

	void Exit();

	float GetDeltaTime() const
	{
		return deltaTime_;
	}

	float GetElapsedTime() const
	{
		return elapsedTime_;
	}

private:
	InputManager* inputManager_;
	ObjectManager* objectManager_;
	Renderer* renderer_;
	WindowManager* windowManager_;
	Editor* editor_;
	CameraManager* cameraManager_;

	Application *application_;
	
	std::vector<ObjectBase *> registeredObjects_;
	std::vector<ObjectBase *> tickableObjects_;

	float deltaTime_;
	float elapsedTime_;
};

#endif
