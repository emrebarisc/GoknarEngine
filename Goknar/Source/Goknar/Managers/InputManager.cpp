#include "pch.h"

#include "InputManager.h"
#include "WindowManager.h"

#include "Goknar/Engine.h"
#include "Goknar/Log.h"

#include "GLFW/glfw3.h"

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

	Vector2i windowSize = engine->GetWindowManager()->GetWindowSize();
	glfwSetCursorPos(window, windowSize.x * 0.5, windowSize.y * 0.5);
}

void InputManager::KeyboardCallback(GLFWwindow *window, int key, int scanCode, int action, int mod)
{
	for(const KeyboardListener &keyboardListener : engine->GetInputManager()->keyboardListeners_)
	{
		keyboardListener(key, scanCode, action, mod);
	}

	switch (action)
	{
		case GLFW_PRESS:
		{
			for (const KeyboardDelegate &pressedKeyDelegate : engine->GetInputManager()->pressedKeyDelegates_[key])
			{
				pressedKeyDelegate();
			}

			break;
		}
		case GLFW_RELEASE:
		{
			for (const KeyboardDelegate &releasedKeyDelegate : engine->GetInputManager()->releasedKeyDelegates_[key])
			{
				releasedKeyDelegate();
			}

			break;
		}
		case GLFW_REPEAT:
		{
			for (const KeyboardDelegate &repeatedKeyDelegate : engine->GetInputManager()->repeatedKeyDelegates_[key])
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
	for (const CursorPositionDelegate &cursorDelegate : engine->GetInputManager()->cursorDelegates_)
	{
		cursorDelegate(xPosition, yPosition);
	}
}

void InputManager::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:
	{
		for (const MouseDelegate &pressedMouseDelegate : engine->GetInputManager()->pressedMouseDelegates_[button])
		{
			pressedMouseDelegate();
		}

		break;
	}
	case GLFW_RELEASE:
	{
		for (const MouseDelegate &releasedMouseDelegate : engine->GetInputManager()->releasedMouseDelegates_[button])
		{
			releasedMouseDelegate();
		}

		break;
	}
	case GLFW_REPEAT:
	{
		for (const MouseDelegate &repeatedMouseDelegate : engine->GetInputManager()->repeatedMouseDelegates_[button])
		{
			repeatedMouseDelegate();
		}

		break;
	}
	default:
		break;
	}
}

void InputManager::ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	for (const ScrollDelegate &scrollDelegate : engine->GetInputManager()->scrollDelegates_)
	{
		scrollDelegate(xOffset, yOffset);
	}
}

void InputManager::CharCallback(GLFWwindow * window, unsigned int codePoint)
{
	for (const CharDelegate &charDelegate : engine->GetInputManager()->charDelegates_)
	{
		charDelegate(codePoint);
	}
}
