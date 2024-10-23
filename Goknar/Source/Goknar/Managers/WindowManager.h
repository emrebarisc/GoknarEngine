#ifndef __WINDOWMANAGER_H__
#define __WINDOWMANAGER_H__

#include <functional>

#include "Goknar/Core.h"
#include "Goknar/Delegates/MulticastDelegate.h"
#include "Goknar/Math/GoknarMath.h"

struct GLFWwindow;
struct GLFWmonitor;

class GOKNAR_API WindowManager
{
	friend class Engine;

public:
	WindowManager();
	~WindowManager();

	static inline void WindowSizeCallback(GLFWwindow *window, int w, int h);

	void PreInit();
	void Init();
	void PostInit();
	
	inline GLFWwindow* GetMainWindow() const
	{
		return mainWindow_;
	}

	void Update();
	void UpdateWindow();
	void UpdateViewport();

	void SetWindowWidth(int w);
	void SetWindowHeight(int h);
	void SetWindowTitle(const char *title);
	void SetMSAA(int MSAAValue);
	void SetContextVersion(int major, int minor);
	void SetOpenGLProfile(int profile1, int profile2);
	void SetVSync(bool isEnable);

	void SetIsInFullscreen(bool isInFullscreen);

	bool GetIsInFullscreen() const
	{
		return isInFullscreen_;
	}

	void HandleFullscreenState();
	void ToggleFullscreen();

	GLFWwindow* CreateNewWindow(int width, int height, const char* title, GLFWwindow* shareContextWith = nullptr, bool isInFullscreen = false);
	bool GetWindowShouldBeClosed(GLFWwindow* window = nullptr);
	void CloseWindow(GLFWwindow* window = nullptr);

	void SetWindowSize(int w, int h);
	Vector2i GetWindowSize() const
	{
		return Vector2i(windowWidth_, windowHeight_);
	}

	void HandleWindowSizeChange();

	void AddWindowSizeCallback(const Delegate<void(int, int)>& callback)
	{
		windowSizeDelegate_ += callback;
	}

private:
	static void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);

	void SetWindowSize_Impl(int w, int h);

	GLFWwindow* mainWindow_;
	GLFWmonitor* mainMonitor_;
	const char* windowTitle_;

	MulticastDelegate<void(int, int)> windowSizeDelegate_;
	
	int MSAAValue_;

	int contextVersionMajor_;
	int contextVersionMinor_;

	int windowWidth_, windowHeight_;

	bool isInFullscreen_;
};

#endif