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

	void SetWindowWidth(int w, bool updateViewport = true);
	void SetWindowHeight(int h, bool updateViewport = true);
	void SetWindowTitle(const char *title);
	void SetMSAA(int MSAAValue);
	void SetContextVersion(int major, int minor);
	void SetOpenGLProfile(int profile1, int profile2);
	bool SetVSync(bool isEnable);
	bool GetIsVSyncChangeSupported() const
	{
		return isVSyncChangeSupported_;
	}
	bool GetIsVSyncEnabled() const
	{
		return isVSyncEnabled_;
	}

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

	void SetWindowSize(int w, int h, bool updateViewport = true);
	Vector2i GetWindowSize() const
	{
		return Vector2i(windowWidth_, windowHeight_);
	}
	Vector2i GetFramebufferSize() const
	{
		return Vector2i(framebufferWidth_, framebufferHeight_);
	}

	void HandleWindowSizeChange(bool updateViewport = true);

	void AddWindowSizeCallback(const Delegate<void(int, int)>& callback)
	{
		windowSizeDelegate_ += callback;
	}

	void RemoveWindowSizeCallback(const Delegate<void(int, int)>& callback)
	{
		windowSizeDelegate_ -= callback;
	}

private:
	static void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);

	bool QueryVSyncChangeSupport() const;
	void SetWindowSize_Impl(int w, int h);

	bool IsGLFWExtensionAvailable(const char* extensionName) const;
	bool IsGLFWProcAddressAvailable(const char* functionName) const;

	GLFWwindow* mainWindow_;
	GLFWmonitor* mainMonitor_;
	const char* windowTitle_;

	MulticastDelegate<void(int, int)> windowSizeDelegate_;
	
	int MSAAValue_;

	int contextVersionMajor_;
	int contextVersionMinor_;

	int windowWidth_, windowHeight_;
	int framebufferWidth_, framebufferHeight_;

	bool isInFullscreen_;
	bool isVSyncChangeSupported_;
	bool isVSyncEnabled_;
};

#endif
