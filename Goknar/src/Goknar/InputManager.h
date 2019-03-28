/*
*	Game Engine Project
*	Emre Baris Coskun
*	2018
*/
#ifndef __INPUTMANAGER_H__
#define __INPUTMANAGER_H__

#include "Core.h"

#include <map>

struct GLFWwindow;

class GOKNAR_API InputManager
{
public:
	InputManager();
	~InputManager();

	void Init();

	static inline void KeyboardCallback(GLFWwindow *window, int key, int scanCode, int action, int mod);

	void SetKeyIsPressed(int key);
	void SetKeyIsReleased(int key);
	void SetKeyIsRepeated(int key);

	bool GetKeyIsPressed(int key);
	bool GetKeyIsReleased(int key);
	bool GetKeyIsRepeated(int key);

private:
	std::map< int, bool > pressedKeys_;
	std::map< int, bool > releasedKeys_;
	std::map< int, bool > repeatedKeys_;
};

#endif