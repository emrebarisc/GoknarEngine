/*
*	Game Engine Project
*	Emre Baris Coskun
*	2018
*/

#ifndef __WINDOWMANAGER_H__
#define __WINDOWMANAGER_H__

#include "Core.h"

struct GLFWwindow;
struct GLFWmonitor;

class GOKNAR_API WindowManager
{
public:
	WindowManager();
	~WindowManager();

	void Init();
	
	inline GLFWwindow* GetWindow() const
	{
		return mainWindow_;
	}

	bool GetWindowShouldBeClosed();

	void SetWindowWidth(int w);
	void SetWindowHeight(int h);
	void SetWindowTitle(const char *title);
	void SetMSAA(int MSAAValue);
	void SetContextVersion(int major, int minor);
	void SetOpenGLProfile(int profile1, int profile2);
	void SetVSync(bool isEnable);

	void ToggleFullscreen();

private:
	GLFWwindow *mainWindow_;
	GLFWmonitor *mainMonitor_;
	const char *windowTitle_;
	
	// Defines the multisample anti-aliasing level
	int MSAAValue_;

	int contextVersionMajor_;
	int contextVersionMinor_;

	int windowWidth_, windowHeight_;

	bool isInFullscreen_;
};

#endif