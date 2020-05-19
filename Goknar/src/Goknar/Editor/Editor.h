#pragma once

#include "Goknar/Core.h"

class EditorCameraController;
class WindowManager;

class GOKNAR_API Editor
{
public:
	Editor();
	virtual ~Editor();

	virtual void Init();
	virtual void Tick(float deltaTime) = 0;

protected:
	WindowManager* windowManager_;
	EditorCameraController* editorCameraController;
};
