#include "pch.h"

#include "CameraManager.h"

#include "Goknar/Camera.h"
#include "Goknar/Engine.h"
#include "WindowManager.h"

CameraManager::~CameraManager()
{
	for (Camera* camera : cameras_)
	{
		delete camera;
	}
}

void CameraManager::SetActiveCamera(Camera* camera)
{
	activeCamera_ = camera;
	if (activeCamera_->GetCameraType() == CameraType::Scene)
	{
		engine->GetWindowManager()->UpdateWindow();
	}
}
