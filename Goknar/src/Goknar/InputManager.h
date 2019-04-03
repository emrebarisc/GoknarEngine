#ifndef __INPUTMANAGER_H__
#define __INPUTMANAGER_H__

#include "pch.h"

#include "Core.h"

struct GLFWwindow;

enum class GOKNAR_API INPUT_TYPE : uint8_t
{
	G_KEYBOARD = 0,
	G_CHAR,
	G_MOUSE,
	G_CURSOR,
	G_SCROLL
};

enum class GOKNAR_API INPUT_ACTION : uint8_t
{
	G_PRESS = 0,
	G_RELEASE,
	G_REPEAT
};

typedef std::function<void()> KeyboardDelegate;
typedef std::vector < KeyboardDelegate > KeyboardDelegateVector;

typedef std::function<void(int, int, int, int)> KeyboardListener;
typedef std::vector < KeyboardListener > KeyboardListenerVector;

typedef std::function<void()> MouseDelegate;
typedef std::vector < MouseDelegate > MouseDelegateVector;

typedef std::function<void(double, double)> CursorDelegate;
typedef std::vector < CursorDelegate > CursorDelegateVector;

typedef std::function<void(double, double)> ScrollDelegate;
typedef std::vector < ScrollDelegate > ScrollDelegateVector;

typedef std::function<void(unsigned int)> CharDelegate;
typedef std::vector < CharDelegate > CharDelegateVector;


class GOKNAR_API InputManager
{
public:
	InputManager();
	~InputManager();

	void Init();

	static inline void KeyboardCallback(GLFWwindow *window, int key, int scanCode, int action, int mod);
	static inline void CursorPositionCallback(GLFWwindow *window, double xPosition, double yPosition);
	static inline void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
	static inline void ScrollCallback(GLFWwindow *window, double xOffset, double yOffset);
	static inline void CharCallback(GLFWwindow *window, unsigned int codePoint);

	void AddKeyboardInputDelegate(int keyCode, INPUT_ACTION inputAction, const KeyboardDelegate &binderFunction)
	{
		switch (inputAction)
		{
			case INPUT_ACTION::G_PRESS: 
				pressedKeyDelegates_[keyCode].push_back(binderFunction);
				break;
			case INPUT_ACTION::G_RELEASE:
				releasedKeyDelegates_[keyCode].push_back(binderFunction);
				break;
			case INPUT_ACTION::G_REPEAT:
				repeatedKeyDelegates_[keyCode].push_back(binderFunction);
				break;
			default: ;
		}
	}

	void AddKeyboardListener(const KeyboardListener &keyboardListener)
	{
		keyboardListeners_.push_back(keyboardListener);
	}

	void AddMouseInputDelegate(int keyCode, INPUT_ACTION inputAction, const KeyboardDelegate &binderFunction)
	{
		switch (inputAction)
		{
		case INPUT_ACTION::G_PRESS:
			pressedMouseDelegates_[keyCode].push_back(binderFunction);
			break;
		case INPUT_ACTION::G_RELEASE:
			releasedMouseDelegates_[keyCode].push_back(binderFunction);
			break;
		case INPUT_ACTION::G_REPEAT:
			repeatedMouseDelegates_[keyCode].push_back(binderFunction);
			break;
		default:;
		}
	}

	void AddCursorDelegate(const CursorDelegate &cursorDelegate)
	{
		cursorDelegates_.push_back(cursorDelegate);
	}

	void AddScrollDelegate(const ScrollDelegate &cursorDelegate)
	{
		scrollDelegates_.push_back(cursorDelegate);
	}

	void AddCharDelegate(const CharDelegate &charDelegate)
	{
		charDelegates_.push_back(charDelegate);
	}

private:
	// Keyboard Delegates
	std::unordered_map< int, KeyboardDelegateVector > pressedKeyDelegates_;
	std::unordered_map< int, KeyboardDelegateVector > releasedKeyDelegates_;
	std::unordered_map< int, KeyboardDelegateVector > repeatedKeyDelegates_;

	KeyboardListenerVector keyboardListeners_;

	// Mouse Delegates
	std::unordered_map< int, MouseDelegateVector > pressedMouseDelegates_;
	std::unordered_map< int, MouseDelegateVector > releasedMouseDelegates_;
	std::unordered_map< int, MouseDelegateVector > repeatedMouseDelegates_;

	// Cursor Delegates
	CursorDelegateVector cursorDelegates_;

	// Scroll Delegates
	ScrollDelegateVector scrollDelegates_;

	// Scroll Delegates
	CharDelegateVector charDelegates_;
};

#endif