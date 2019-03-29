#include "pch.h"

#include "InputManager.h"
#include "WindowManager.h"
#include "Engine.h"

#include "GLFW/glfw3.h"
#include "Log.h"

InputManager::InputManager()
{
}

InputManager::~InputManager() = default;

void InputManager::Init()
{
	GLFWwindow *window = engine->GetWindowManager()->GetWindow();

	glfwSetKeyCallback(window, InputManager::KeyboardCallback);
	glfwSetCursorPosCallback(window, InputManager::CursorPositionCallback);
	glfwSetMouseButtonCallback(window, InputManager::MouseButtonCallback);
	glfwSetScrollCallback(window, InputManager::ScrollCallback);
	glfwSetCharCallback(window, InputManager::CharCallback);
}

void InputManager::KeyboardCallback(GLFWwindow *window, int key, int scanCode, int action, int mod)
{
	GOKNAR_CORE_INFO("Key Pressed key: {} scanCode: {} action: {} mod: {}", key, scanCode, action, mod);
	
	switch (action)
	{
		case GLFW_PRESS:
		{
			for (auto &pressedKeyDelegate : engine->GetInputManager()->pressedKeyDelegates_[key])
			{
				pressedKeyDelegate();
			}

			break;
		}
		case GLFW_RELEASE:
		{
			for (auto &releasedKeyDelegate : engine->GetInputManager()->releasedKeyDelegates_[key])
			{
				releasedKeyDelegate();
			}

			break;
		}
		case GLFW_REPEAT:
		{
			for (auto &repeatedKeyDelegate : engine->GetInputManager()->repeatedKeyDelegates_[key])
			{
				repeatedKeyDelegate();
			}

			break;
		}
		default:
			break;
	}
}

void InputManager::CursorPositionCallback(GLFWwindow* window, double xPosition, double yPosition)
{
	GOKNAR_CORE_INFO("Cursor X Position: {} Y Position: {}", xPosition, yPosition);

	for (auto &cursorDelegate : engine->GetInputManager()->cursorDelegates_)
	{
		cursorDelegate(xPosition, yPosition);
	}
}

void InputManager::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	GOKNAR_CORE_INFO("Mouse button click Button Id: {} Action Id: {} Mods: {}", button, action, mods);
}

void InputManager::ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	GOKNAR_CORE_INFO("Mouse Scroll xOffset: {} yOffset: {}", xOffset, yOffset);
}

void InputManager::CharCallback(GLFWwindow * window, unsigned int codePoint)
{
	GOKNAR_CORE_INFO("Character Pressed codePoint: {}", codePoint);
}
