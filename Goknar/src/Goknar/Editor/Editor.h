#pragma once

#include "Goknar/Core.h"

class WindowManager;

class GOKNAR_API Editor
{
public:
	Editor();
	virtual ~Editor();

	virtual void Init() = 0;
	virtual void Tick(float deltaTime) = 0;

protected:
	WindowManager* windowManager_;
};
