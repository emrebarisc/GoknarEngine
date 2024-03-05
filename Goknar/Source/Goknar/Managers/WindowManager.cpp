#include "pch.h"

#include "WindowManager.h"

#include "Goknar/Camera.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Managers/CameraManager.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/InputManager.h"

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
	isInFullscreen_ = false;
	SetContextVersion(4, 6);
}

WindowManager::~WindowManager()
{
	glfwDestroyWindow(mainWindow_);
	glfwTerminate();
}

void WindowManager::WindowSizeCallback(GLFWwindow* window, int w, int h)
{
	engine->GetWindowManager()->SetWindowSize_Impl(w, h);
}

void WindowManager::PreInit()
{
	const int glfwResult = glfwInit();
	GOKNAR_CORE_ASSERT(glfwResult, "GLFW failed to initialize");

	const Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();

	windowWidth_ = activeCamera->GetImageWidth();
	windowHeight_ = activeCamera->GetImageHeight();

	mainMonitor_ = glfwGetPrimaryMonitor();

	GLFWmonitor* windowMonitor = nullptr;
	if (isInFullscreen_)
	{
		windowMonitor = mainMonitor_;
	}

	mainWindow_ = glfwCreateWindow(windowWidth_, windowHeight_, windowTitle_, windowMonitor, 0);

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
	bool enableDebugContext = false;
#ifdef GOKNAR_PLATFORM_WINDOWS
#if defined(GOKNAR_BUILD_DEBUG) 
	enableDebugContext = true;
#endif
#endif
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, enableDebugContext);

	SetMSAA(MSAAValue_);

	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::ESCAPE, INPUT_ACTION::G_PRESS, std::bind(&WindowManager::CloseWindow, this));
	
	glfwSetFramebufferSizeCallback(mainWindow_, FrameBufferSizeCallback);
}

void WindowManager::Init()
{
}

void WindowManager::PostInit()
{
}

bool WindowManager::GetWindowShouldBeClosed()
{
	return glfwWindowShouldClose(mainWindow_);
}

void WindowManager::FrameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
	engine->GetWindowManager()->SetWindowSize_Impl(width, height);
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
		UpdateViewport();
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
		UpdateViewport();
		UpdateWindow();
	}
}

void WindowManager::SetWindowSize(int w, int h)
{
	glfwSetWindowSize(mainWindow_, w, h);
}

void WindowManager::SetWindowSize_Impl(int w, int h)
{
	windowWidth_ = w;
	windowHeight_ = h;

	const std::vector<Camera*>& cameras = engine->GetCameraManager()->GetCameras();
	for(decltype(cameras.begin()) cameraIterator = cameras.begin(); cameraIterator < cameras.end(); ++cameraIterator)
	{
		Camera* camera = *cameraIterator;

		if(camera->GetCameraType() == CameraType::Scene)
		{
			camera->SetImageWidth(w);
			camera->SetImageHeight(h);
			camera->Update();
		}
	}

	UpdateWindow();
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

void WindowManager::HandleFullscreenState()
{
	GLFWmonitor* monitor = nullptr;

	int positionX = 0;
	int positionY = 0; // Small value to show window title bar on non-fullscreen mode
	float w = windowWidth_;
	float h = windowHeight_;
	float refreshRate = GLFW_DONT_CARE;

	if (isInFullscreen_)
	{
		monitor = mainMonitor_;

		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		w = mode->width;
		h = mode->height;
		refreshRate = mode->refreshRate;
	}

	glfwSetWindowMonitor(mainWindow_, monitor, positionX, positionY, w, h, refreshRate);
}

void WindowManager::ToggleFullscreen()
{
	SetIsInFullscreen(!isInFullscreen_);
	HandleFullscreenState();
}

void WindowManager::Update()
{
	glfwPollEvents();
	glfwSwapBuffers(mainWindow_);
}

void WindowManager::UpdateWindow()
{
	windowSizeDelegate_(windowWidth_, windowHeight_);
}

void WindowManager::UpdateViewport()
{
	Camera* activeCamera = engine->GetCameraManager()->GetActiveCamera();
	if (activeCamera)
	{
		glViewport(0, 0, activeCamera->GetImageWidth(), activeCamera->GetImageHeight());
	}
}
