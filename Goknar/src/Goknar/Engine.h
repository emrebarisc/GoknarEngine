/*
*	Game Engine Project
*	Emre Baris Coskun
*	2018
*/

#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <vector>

#include "Application.h"

class Editor;
class GraphicsManager;
class InputManager;
class ObjectBase;
class ObjectManager;
class Renderer;
class WindowManager;

// Global Engine variable
extern class Engine *engine;

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

	GraphicsManager* GetGraphicsManager() const
	{
		return graphicsManager_;
	}

	Renderer* GetRenderer() const
	{
		return renderer_;
	}

	Editor* GetEditor() const
	{
		return editor_;
	}

	void SetApplication(Application* application);

	void RegisterObject(ObjectBase *object);
	void AddToTickableObjects(ObjectBase *object);

	void Exit();

private:
	GraphicsManager *graphicsManager_;
	InputManager *inputManager_;
	ObjectManager *objectManager_;
	Renderer *renderer_;
	WindowManager *windowManager_;
	Editor *editor_;

	Application *application_;

	std::vector<ObjectBase *> registeredObjects_;
	std::vector<ObjectBase *> tickableObjects_;
};

#endif
