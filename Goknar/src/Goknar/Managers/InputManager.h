#ifndef __INPUTMANAGER_H__
#define __INPUTMANAGER_H__

#include "pch.h"

#include "GLFW/glfw3.h"

#include "Goknar/Core.h"

struct GLFWwindow;

enum class GOKNAR_API INPUT_TYPE : uint8_t
{
	G_KEYBOARD = 0x00,
	G_CHAR,
	G_MOUSE,
	G_CURSOR,
	G_SCROLL
};

enum class GOKNAR_API INPUT_ACTION : uint8_t
{
	G_PRESS = 0x00,
	G_RELEASE,
	G_REPEAT
};

enum class GOKNAR_API KEY_MAP : int
{
	UNKNOWN = GLFW_KEY_UNKNOWN,
	SPACE = GLFW_KEY_SPACE,
	APOSTROPHE = GLFW_KEY_APOSTROPHE,
	COMMA = GLFW_KEY_COMMA,
	MINUS = GLFW_KEY_MINUS,
	PERIOD = GLFW_KEY_PERIOD,
	SLASH = GLFW_KEY_SLASH,
	NUM_0 = GLFW_KEY_0,
	NUM_1 = GLFW_KEY_1,
	NUM_2 = GLFW_KEY_2,
	NUM_3 = GLFW_KEY_3,
	NUM_4 = GLFW_KEY_4,
	NUM_5 = GLFW_KEY_5,
	NUM_6 = GLFW_KEY_6,
	NUM_7 = GLFW_KEY_7,
	NUM_8 = GLFW_KEY_8,
	NUM_9 = GLFW_KEY_9,
	SEMICOLON = GLFW_KEY_SEMICOLON,
	EQUAL = GLFW_KEY_EQUAL,
	A = GLFW_KEY_A,
	B = GLFW_KEY_B,
	C = GLFW_KEY_C,
	D = GLFW_KEY_D,
	E = GLFW_KEY_E,
	F = GLFW_KEY_F,
	G = GLFW_KEY_G,
	H = GLFW_KEY_H,
	I = GLFW_KEY_I,
	J = GLFW_KEY_J,
	K = GLFW_KEY_K,
	L = GLFW_KEY_L,
	M = GLFW_KEY_M,
	N = GLFW_KEY_N,
	O = GLFW_KEY_O,
	P = GLFW_KEY_P,
	Q = GLFW_KEY_Q,
	R = GLFW_KEY_R,
	S = GLFW_KEY_S,
	T = GLFW_KEY_T,
	U = GLFW_KEY_U,
	V = GLFW_KEY_V,
	W = GLFW_KEY_W,
	X = GLFW_KEY_X,
	Y = GLFW_KEY_Y,
	Z = GLFW_KEY_Z,
	LEFT_BRACKET = GLFW_KEY_LEFT_BRACKET,
	BACKSLASH = GLFW_KEY_BACKSLASH,
	RIGHT_BRACKET = GLFW_KEY_RIGHT_BRACKET,
	GRAVE_ACCENT = GLFW_KEY_GRAVE_ACCENT,
	WORLD_1 = GLFW_KEY_WORLD_1,
	WORLD_2 = GLFW_KEY_WORLD_2,
	ESCAPE = GLFW_KEY_ESCAPE,
	ENTER = GLFW_KEY_ENTER,
	TAB = GLFW_KEY_TAB,
	BACKSPACE = GLFW_KEY_BACKSPACE,
	INSERT = GLFW_KEY_INSERT,
	DLT = GLFW_KEY_DELETE,
	RIGHT = GLFW_KEY_RIGHT,
	LEFT = GLFW_KEY_LEFT,
	DOWN = GLFW_KEY_DOWN,
	UP = GLFW_KEY_UP,
	PAGE_UP = GLFW_KEY_PAGE_UP,
	PAGE_DOWN = GLFW_KEY_PAGE_DOWN,
	HOME = GLFW_KEY_HOME,
	END = GLFW_KEY_END,
	CAPS_LOCK = GLFW_KEY_CAPS_LOCK,
	SCROLL_LOCK = GLFW_KEY_SCROLL_LOCK,
	NUM_LOCK = GLFW_KEY_NUM_LOCK,
	PRINT_SCREEN = GLFW_KEY_PRINT_SCREEN,
	PAUSE = GLFW_KEY_PAUSE,
	F1 = GLFW_KEY_F1,
	F2 = GLFW_KEY_F2,
	F3 = GLFW_KEY_F3,
	F4 = GLFW_KEY_F4,
	F5 = GLFW_KEY_F5,
	F6 = GLFW_KEY_F6,
	F7 = GLFW_KEY_F7,
	F8 = GLFW_KEY_F8,
	F9 = GLFW_KEY_F9,
	F10 = GLFW_KEY_F10,
	F11 = GLFW_KEY_F11,
	F12 = GLFW_KEY_F12,
	F13 = GLFW_KEY_F13,
	F14 = GLFW_KEY_F14,
	F15 = GLFW_KEY_F15,
	F16 = GLFW_KEY_F16,
	F17 = GLFW_KEY_F17,
	F18 = GLFW_KEY_F18,
	F19 = GLFW_KEY_F19,
	F20 = GLFW_KEY_F20,
	F21 = GLFW_KEY_F21,
	F22 = GLFW_KEY_F22,
	F23 = GLFW_KEY_F23,
	F24 = GLFW_KEY_F24,
	F25 = GLFW_KEY_F25,
	KP_0 = GLFW_KEY_KP_0,
	KP_1 = GLFW_KEY_KP_1,
	KP_2 = GLFW_KEY_KP_2,
	KP_3 = GLFW_KEY_KP_3,
	KP_4 = GLFW_KEY_KP_4,
	KP_5 = GLFW_KEY_KP_5,
	KP_6 = GLFW_KEY_KP_6,
	KP_7 = GLFW_KEY_KP_7,
	KP_8 = GLFW_KEY_KP_8,
	KP_9 = GLFW_KEY_KP_9,
	KP_DECIMAL = GLFW_KEY_KP_DECIMAL,
	KP_DIVIDE = GLFW_KEY_KP_DIVIDE,
	KP_MULTIPLY = GLFW_KEY_KP_MULTIPLY,
	KP_SUBTRACT = GLFW_KEY_KP_SUBTRACT,
	KP_ADD = GLFW_KEY_KP_ADD,
	KP_ENTER = GLFW_KEY_KP_ENTER,
	KP_EQUAL = GLFW_KEY_KP_EQUAL,
	LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,
	LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL,
	LEFT_ALT = GLFW_KEY_LEFT_ALT,
	LEFT_SUPER = GLFW_KEY_LEFT_SUPER,
	RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT,
	RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL,
	RIGHT_ALT = GLFW_KEY_RIGHT_ALT,
	RIGHT_SUPER = GLFW_KEY_RIGHT_SUPER,
	MENU = GLFW_KEY_MENU,
	LAST = MENU
};

typedef std::function<void()> KeyboardDelegate;
typedef std::vector < KeyboardDelegate > KeyboardDelegateVector;

typedef std::function<void(int, int, int, int)> KeyboardListener;
typedef std::vector < KeyboardListener > KeyboardListenerVector;

typedef std::function<void()> MouseDelegate;
typedef std::vector < MouseDelegate > MouseDelegateVector;

typedef std::function<void(double, double)> CursorPositionDelegate;
typedef std::vector < CursorPositionDelegate > CursorDelegateVector;

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

	void AddKeyboardInputDelegate(KEY_MAP keyCode, INPUT_ACTION inputAction, const KeyboardDelegate &binderFunction)
	{
		switch (inputAction)
		{
			case INPUT_ACTION::G_PRESS: 
				pressedKeyDelegates_[(int)keyCode].push_back(binderFunction);
				break;
			case INPUT_ACTION::G_RELEASE:
				releasedKeyDelegates_[(int)keyCode].push_back(binderFunction);
				break;
			case INPUT_ACTION::G_REPEAT:
				repeatedKeyDelegates_[(int)keyCode].push_back(binderFunction);
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

	void AddCursorDelegate(const CursorPositionDelegate &cursorDelegate)
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
	std::unordered_map< int, KeyboardDelegateVector > repeatedKeyDelegates_;
	std::unordered_map< int, KeyboardDelegateVector > releasedKeyDelegates_;

	KeyboardListenerVector keyboardListeners_;

	// Mouse Delegates
	std::unordered_map< int, MouseDelegateVector > pressedMouseDelegates_;
	std::unordered_map< int, MouseDelegateVector > repeatedMouseDelegates_;
	std::unordered_map< int, MouseDelegateVector > releasedMouseDelegates_;

	// Cursor Delegates
	CursorDelegateVector cursorDelegates_;

	// Scroll Delegates
	ScrollDelegateVector scrollDelegates_;

	// Char Delegates
	CharDelegateVector charDelegates_;
};

#endif