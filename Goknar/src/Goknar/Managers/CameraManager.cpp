#include "pch.h"

#include "CameraManager.h"

#include "Goknar/Camera.h"

CameraManager::CameraManager()
{
	Camera* defaultCamera = new Camera();

	activeCamera_ = defaultCamera;
	cameras_.push_back(defaultCamera);
}
