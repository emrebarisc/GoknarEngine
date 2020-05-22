#pragma once

#include "Goknar/Core.h"

class EditorCameraController;
class EditorGroundGrid;
class WindowManager;

class GOKNAR_API Editor
{
public:
	Editor();
	virtual ~Editor();

	virtual void Init();
	virtual void Tick(float deltaTime) = 0;

protected:
	EditorCameraController* editorCameraController_;
	EditorGroundGrid* editorGroundGrid_;
	WindowManager* windowManager_;
};
