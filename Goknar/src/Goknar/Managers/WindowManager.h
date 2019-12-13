#ifndef __WINDOWMANAGER_H__
#define __WINDOWMANAGER_H__

#include "Goknar/Core.h"
#include "Goknar/Math.h"

struct GLFWwindow;
struct GLFWmonitor;

class GOKNAR_API WindowManager
{
	friend class Engine;

public:
	WindowManager();
	~WindowManager();

	static inline void WindowSizeCallback(GLFWwindow *window, int w, int h);

	void Init();
	
	inline GLFWwindow* GetWindow() const
	{
		return mainWindow_;
	}

	void Update();

	void SetWindowWidth(int w);
	void SetWindowHeight(int h);
	void SetWindowTitle(const char *title);
	void SetMSAA(int MSAAValue);
	void SetContextVersion(int major, int minor);
	void SetOpenGLProfile(int profile1, int profile2);
	void SetVSync(bool isEnable);

	void ToggleFullscreen();

	bool GetWindowShouldBeClosed();

	void SetWindowSize(int w, int h)
	{
		windowWidth_ = w;
		windowHeight_ = h;
	}

	Vector2i GetWindowSize() const
	{
		return Vector2i(windowWidth_, windowHeight_);
	}

private:
	void CloseWindow();

	static void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);

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