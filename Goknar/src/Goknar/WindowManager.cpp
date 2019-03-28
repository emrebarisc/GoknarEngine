/*
*	Game Engine Project
*	Emre Baris Coskun
*	2018
*/
#include "pch.h"

#include "WindowManager.h"

#include "Core.h"
#include "Log.h"

//#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <iostream>

WindowManager::WindowManager()
{
	windowWidth_ = 1024;
	windowHeight_ = 768;
	windowTitle_ = "Goknar Engine";
	mainMonitor_ = nullptr;
	MSAAValue_ = 0;
	isInFullscreen_ = false;
}

WindowManager::~WindowManager()
{
	glfwDestroyWindow(mainWindow_);
	glfwTerminate();
}

void WindowManager::Init()
{
	int glfwResult = glfwInit();
	GOKNAR_CORE_ASSERT(glfwResult, "GLFW failed to initialize");

	mainWindow_ = glfwCreateWindow(windowWidth_, windowHeight_, windowTitle_, mainMonitor_, 0);

	glfwSetInputMode(mainWindow_, GLFW_STICKY_KEYS, GL_TRUE);
	SetVSync(true);
	glfwMakeContextCurrent(mainWindow_);
	/*glewExperimental = true;

	int glfwResult = glewInit();
	GOKNAR_CORE_ASSERT(glfwResult, "Failed to initialize GLEW.");*/

	if (!mainWindow_)
	{
		std::cerr << "Window could not be created.\n" << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
}

bool WindowManager::GetWindowShouldBeClosed()
{
	return glfwWindowShouldClose(mainWindow_);
}

void WindowManager::SetWindowWidth(int w)
{
	windowWidth_ = w;
}

void WindowManager::SetWindowHeight(int h)
{
	windowHeight_ = h;
}

void WindowManager::SetWindowTitle(const char *title)
{
	windowTitle_ = title;
}

void WindowManager::SetMSAA(int MMAAValue)
{
	MSAAValue_ = MMAAValue;
	glfwWindowHint(GLFW_SAMPLES, MMAAValue);
}

void WindowManager::SetContextVersion(int major, int minor)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
}

void WindowManager::SetOpenGLProfile(int profile1, int profile2)
{
	glfwWindowHint(profile1, profile2);
}

void WindowManager::SetVSync(bool isEnable)
{
	glfwSwapInterval(isEnable ? 1 : 0);
}

void WindowManager::ToggleFullscreen()
{
/*
	if (!isInFullscreen_)
	{
		mainMonitor_ = glfwGetPrimaryMonitor();
		glfwSetWindowMonitor(mainWindow_, mainMonitor_, 0, 0, windowWidth_, windowHeight_, 60);
	}

	isInFullscreen_ = !isInFullscreen_;
*/
}
