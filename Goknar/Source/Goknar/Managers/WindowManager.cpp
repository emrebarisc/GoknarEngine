#include "pch.h"

#include "WindowManager.h"

#include "Goknar/Camera.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Engine.h"
#include "InputManager.h"
#include "Goknar/Log.h"

#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

WindowManager::WindowManager()
{
	windowWidth_ = 1024;
	windowHeight_ = 768;
	windowTitle_ = "Goknar Engine";
	mainMonitor_ = nullptr;
	MSAAValue_ = 4;
	isInFullscreen_ = true;
	SetContextVersion(4, 6);
}

WindowManager::~WindowManager()
{
	glfwDestroyWindow(mainWindow_);
	glfwTerminate();
}

void WindowManager::WindowSizeCallback(GLFWwindow* window, int w, int h)
{
	engine->GetWindowManager()->SetWindowSize(w, h);
}

void WindowManager::Init()
{
	const int glfwResult = glfwInit();
	GOKNAR_CORE_ASSERT(glfwResult, "GLFW failed to initialize");

	const Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();

	windowWidth_ = activeCamera->GetImageWidth();
	windowHeight_ = activeCamera->GetImageHeight();

	mainWindow_ = glfwCreateWindow(windowWidth_, windowHeight_, windowTitle_, mainMonitor_, 0);

	if(mainWindow_)
	{
		glfwSetWindowSizeCallback(mainWindow_, WindowSizeCallback);

		glfwSetInputMode(mainWindow_, GLFW_STICKY_KEYS, GL_TRUE);
		SetVSync(true);
		glfwMakeContextCurrent(mainWindow_);

		const int gladResult = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		GOKNAR_CORE_ASSERT(gladResult, "Failed to initialize GLAD!.");
	}
	else
	{
		GOKNAR_CORE_FATAL("Window could not be created.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	SetMSAA(MSAAValue_);

	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::ESCAPE, INPUT_ACTION::G_PRESS, std::bind(&WindowManager::CloseWindow, this));
	
	glfwSetFramebufferSizeCallback(mainWindow_, FrameBufferSizeCallback);
}

bool WindowManager::GetWindowShouldBeClosed()
{
	return glfwWindowShouldClose(mainWindow_);
}

void WindowManager::FrameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
	engine->GetWindowManager()->SetWindowWidth(width);
	engine->GetWindowManager()->SetWindowHeight(height);

	Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	activeCamera->SetImageWidth(width);
	activeCamera->SetImageHeight(height);
	activeCamera->Update();
}

void WindowManager::CloseWindow()
{
	glfwSetWindowShouldClose(mainWindow_, true);
}

void WindowManager::SetWindowWidth(int w)
{
	windowWidth_ = w;
	Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	if (activeCamera && activeCamera->GetCameraType() == CameraType::Scene)
	{
		activeCamera->SetImageWidth(windowWidth_);
		UpdateWindow();
	}
}

void WindowManager::SetWindowHeight(int h)
{
	windowHeight_ = h;
	Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	if (activeCamera && activeCamera->GetCameraType() == CameraType::Scene)
	{
		activeCamera->SetImageHeight(windowHeight_);
		UpdateWindow();
	}
}

void WindowManager::SetWindowTitle(const char *title)
{
	windowTitle_ = title;
	glfwSetWindowTitle(mainWindow_, windowTitle_);
}

void WindowManager::SetMSAA(int MSAAValue)
{
	if (MSAAValue == 0)
	{
		glDisable(GL_MULTISAMPLE);
	}
	else
	{
		glEnable(GL_MULTISAMPLE);
	}

	MSAAValue_ = MSAAValue;
	glfwWindowHint(GLFW_SAMPLES, MSAAValue);

	GOKNAR_CORE_INFO("MSAA Value is set to {}", MSAAValue_);
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
	if (!isInFullscreen_)
	{
		glfwSetWindowMonitor(mainWindow_, mainMonitor_, 0, 0, windowWidth_, windowHeight_, GLFW_DONT_CARE);
	}

	isInFullscreen_ = !isInFullscreen_;
}

void WindowManager::Update()
{
	glfwPollEvents();
	glfwSwapBuffers(mainWindow_);
}

void WindowManager::UpdateWindow()
{
	Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	if (activeCamera)
	{
		glViewport(0, 0, activeCamera->GetImageWidth(), activeCamera->GetImageHeight());
	}
}