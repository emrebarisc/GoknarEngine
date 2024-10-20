#include "pch.h"

#include "InputManager.h"
#include "WindowManager.h"

#include "Goknar/Engine.h"
#include "Goknar/Log.h"

#include "GLFW/glfw3.h"

InputManager::KeyboardEvent::KeyboardEvent(KEY_MAP keyCode, const KeyboardDelegate& pressedCallback, const KeyboardDelegate& releasedCallback) :
		keyCode_(keyCode),
		latestAction_(INPUT_ACTION::G_NONE),
		pressedCallback_(pressedCallback),
		releasedCallback_(releasedCallback)
{
}

InputManager::KeyboardEvent::KeyboardEvent(const KeyboardEvent* other) :
	keyCode_(other->keyCode_),
	latestAction_(other->latestAction_),
	pressedCallback_(other->pressedCallback_),
	releasedCallback_(other->releasedCallback_)
{

}

InputManager::KeyboardEvent::~KeyboardEvent()
{

}

void InputManager::KeyboardEvent::OnPressed()
{
	if(latestAction_ != INPUT_ACTION::G_PRESS)
	{
		pressedCallback_();
		latestAction_ = INPUT_ACTION::G_PRESS;
	}
}

void InputManager::KeyboardEvent::OnReleased()
{
	if(latestAction_ != INPUT_ACTION::G_RELEASE)
	{
		releasedCallback_();
		latestAction_ = INPUT_ACTION::G_RELEASE;
	}
}


InputManager::InputManager()
{
	keyboardEvents_ = new KeyboardEventMap();
}

InputManager::~InputManager()
{
	delete keyboardEvents_;
}

void InputManager::PreInit()
{
	window_ = engine->GetWindowManager()->GetWindow();

	glfwSetKeyCallback(window_, InputManager::KeyboardCallback);
	glfwSetCursorPosCallback(window_, InputManager::CursorPositionCallback);
	glfwSetMouseButtonCallback(window_, InputManager::MouseButtonCallback);
	glfwSetScrollCallback(window_, InputManager::ScrollCallback);
	glfwSetCharCallback(window_, InputManager::CharCallback);

	Vector2i windowSize = engine->GetWindowManager()->GetWindowSize();
	SetCursorPosition(windowSize.x * 0.5, windowSize.y * 0.5, window_);
}

void InputManager::Init()
{
}

void InputManager::PostInit()
{
}

void InputManager::KeyboardCallback(GLFWwindow* window, int key, int scanCode, int action, int mod)
{
	InputManager* inputManager = engine->GetInputManager();

	if(!inputManager->keyboardListeners_.isNull())
	{
		inputManager->keyboardListeners_(key, scanCode, action, mod);
	}

	switch (action)
	{
		case GLFW_PRESS:
		{
			if(	inputManager->pressedKeyDelegates_.find(key) != inputManager->pressedKeyDelegates_.end() && 
				!inputManager->pressedKeyDelegates_[key].isNull())
			{
				inputManager->pressedKeyDelegates_[key]();
			}
			
			break;
		}
		case GLFW_RELEASE:
		{
			if(	inputManager->releasedKeyDelegates_.find(key) != inputManager->releasedKeyDelegates_.end() && 
				!inputManager->releasedKeyDelegates_[key].isNull())
			{
				inputManager->releasedKeyDelegates_[key]();
			}

			break;
		}
		case GLFW_REPEAT:
		{
			if(	inputManager->repeatedKeyDelegates_.find(key) != inputManager->repeatedKeyDelegates_.end() && 
				!inputManager->repeatedKeyDelegates_[key].isNull())
			{
				inputManager->repeatedKeyDelegates_[key]();
			}

			break;
		}
		default:
			break;
	}
}

void InputManager::CursorPositionCallback(GLFWwindow* window, double xPosition, double yPosition)
{
	InputManager* inputManager = engine->GetInputManager();
	if(!inputManager->cursorDelegates_.isNull())
	{
		inputManager->cursorDelegates_(xPosition, yPosition);
	}
}

void InputManager::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	InputManager* inputManager = engine->GetInputManager();

	switch (action)
	{
	case GLFW_PRESS:
	{
		if( inputManager->pressedMouseDelegates_.find(button) != inputManager->pressedMouseDelegates_.end() &&
			!inputManager->pressedMouseDelegates_[button].isNull())
		{
			inputManager->pressedMouseDelegates_[button]();
		}
		break;
	}
	case GLFW_RELEASE:
	{
		if( inputManager->releasedMouseDelegates_.find(button) != inputManager->releasedMouseDelegates_.end() &&
			!inputManager->releasedMouseDelegates_[button].isNull())
		{
			inputManager->releasedMouseDelegates_[button]();
		}
		break;
	}
	case GLFW_REPEAT:
	{
		if( inputManager->repeatedMouseDelegates_.find(button) != inputManager->repeatedMouseDelegates_.end() &&
			!inputManager->repeatedMouseDelegates_[button].isNull())
		{
			inputManager->repeatedMouseDelegates_[button]();
		}
		break;
	}
	default:
		break;
	}
}

void InputManager::ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	InputManager* inputManager = engine->GetInputManager();
	if(!inputManager->scrollDelegates_.isNull())
	{
		inputManager->scrollDelegates_(xOffset, yOffset);
	}
}

void InputManager::CharCallback(GLFWwindow * window, unsigned int codePoint)
{
	InputManager* inputManager = engine->GetInputManager();
	if(!inputManager->charDelegates_.isNull())
	{
		inputManager->charDelegates_(codePoint);
	}
}

void InputManager::SetIsCursorVisible(bool isCursorVisible)
{
	if (isCursorVisible)
	{
		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	else
	{
		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}
}

void InputManager::SetCursorPosition(float x, float y, GLFWwindow* window)
{
	if (window == nullptr)
	{
		window = engine->GetWindowManager()->GetWindow();
	}

	glfwSetCursorPos(window, x, y);
}

void InputManager::AddKeyboardEvent(KEY_MAP keyCode, const KeyboardDelegate& pressedCallback, const KeyboardDelegate& releasedCallback)
{
	std::unique_ptr<InputManager::KeyboardEvent> keyboardEvent = 
		std::make_unique<InputManager::KeyboardEvent>(keyCode, pressedCallback, releasedCallback);

	if(keyboardEvents_->find(keyCode) == keyboardEvents_->end())
	{
		AddKeyboardInputDelegate(keyCode, INPUT_ACTION::G_PRESS, pressedCallback);

		AddKeyboardInputDelegate(keyCode, INPUT_ACTION::G_RELEASE, releasedCallback);

		keyboardEvents_->insert(std::pair<KEY_MAP, KeyboardEvents>(keyCode, KeyboardEvents()));
	}

	keyboardEvents_->at(keyCode).push_back(std::move(keyboardEvent));
}

void InputManager::RemoveKeyboardEvent(KEY_MAP keyCode, const KeyboardDelegate& pressedCallback, const KeyboardDelegate& releasedCallback)
{
	if(keyboardEvents_->find(keyCode) == keyboardEvents_->end())
	{
		return;
	}

	KeyboardEvents& keyboardEvents = keyboardEvents_->at(keyCode);

	decltype(keyboardEvents.cbegin()) keyboardEventsIterator = keyboardEvents.cbegin();
	while(keyboardEventsIterator != keyboardEvents.cend())
	{
		const std::unique_ptr<KeyboardEvent>& keyboardEvent = *keyboardEventsIterator;

		if(keyboardEvent->GetPressedCallback() == pressedCallback && keyboardEvent->GetReleasedCallback() == releasedCallback)
		{
			keyboardEvents.erase(keyboardEventsIterator);
			break;
		}

		++keyboardEventsIterator;
	}
}