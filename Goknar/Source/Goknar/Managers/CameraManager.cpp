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

void CameraManager::PreInit()
{
	std::vector<Camera*>::iterator cameraIterator = cameras_.begin();
	for (; cameraIterator != cameras_.end(); ++cameraIterator)
	{
		(*cameraIterator)->Init();
	}
}

void CameraManager::Init()
{
}

void CameraManager::PostInit()
{
}

void CameraManager::AddCamera(Camera* camera)
{
	cameras_.push_back(camera);

	if (cameras_.size() == 1)
	{
		activeCamera_ = camera;
	}
}

void CameraManager::SetActiveCamera(Camera* camera)
{
	activeCamera_ = camera;
	engine->GetWindowManager()->UpdateViewport();
}
