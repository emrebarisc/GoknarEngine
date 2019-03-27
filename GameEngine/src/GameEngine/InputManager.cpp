#include "pch.h"

#include "InputManager.h"
#include "WindowManager.h"
#include "Engine.h"

#include "GLFW/glfw3.h"

InputManager::InputManager()
{
}

InputManager::~InputManager() = default;

void InputManager::Init()
{
	glfwSetKeyCallback(engine->GetWindowManager()->GetWindow(), InputManager::KeyboardCallback);
}

void InputManager::KeyboardCallback(GLFWwindow *window, const int key, int scanCode, const int action, int mod)
{
	switch (action)
	{
		case GLFW_PRESS:
		{
			engine->GetInputManager()->SetKeyIsPressed(key);

			break;
		}
		case GLFW_RELEASE:
		{
			engine->GetInputManager()->SetKeyIsReleased(key);

			break;
		}
		case GLFW_REPEAT:
		{
			engine->GetInputManager()->SetKeyIsRepeated(key);

			break;
		}
		default:
			break;
	}
}

void InputManager::SetKeyIsPressed(const int key)
{
	pressedKeys_[key] = true;
	releasedKeys_[key] = false;
}

void InputManager::SetKeyIsReleased(const int key)
{
	pressedKeys_[key] = false;
	releasedKeys_[key] = true;
	repeatedKeys_[key] = false;
}

void InputManager::SetKeyIsRepeated(const int key)
{
	repeatedKeys_[key] = true;
}

bool InputManager::GetKeyIsPressed(const int key)
{
	return pressedKeys_[key];
}

bool InputManager::GetKeyIsReleased(const int key)
{
	return releasedKeys_[key];
}

bool InputManager::GetKeyIsRepeated(const int key)
{
	return repeatedKeys_[key];
}
