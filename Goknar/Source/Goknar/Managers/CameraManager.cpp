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
	HandleNewlyAddedCameras();
}

void CameraManager::Init()
{
}

void CameraManager::PostInit()
{
}

void CameraManager::HandleNewlyAddedCameras()
{
	if(camerasToAddNextFrameCount_ == 0)
	{
		return;
	}

	std::vector<Camera*>::const_iterator cameraIterator = camerasToAddNextFrame_.cbegin();
	while(cameraIterator != camerasToAddNextFrame_.cend())
	{
		Camera* camera = *cameraIterator;

		camera->Init();

		cameras_.push_back(camera);

		if (activeCamera_ == nullptr && camera->GetCameraType() == CameraType::Scene)
		{
			SetActiveCamera(camera);
		}

		++cameraIterator;
	}

	camerasToAddNextFrame_.clear();
	camerasToAddNextFrameCount_ = 0;
}

void CameraManager::AddCamera(Camera* camera)
{
	camerasToAddNextFrame_.push_back(camera);
	++camerasToAddNextFrameCount_;
}

void CameraManager::RemoveCamera(Camera* camera)
{
	std::vector<Camera*>::const_iterator camerasToAddNextFrameIterator = camerasToAddNextFrame_.cbegin();
	while(camerasToAddNextFrameIterator != camerasToAddNextFrame_.cend())
	{
		if(camera == *camerasToAddNextFrameIterator)
		{
			camerasToAddNextFrame_.erase(camerasToAddNextFrameIterator);
			--camerasToAddNextFrameCount_;
		}
		++camerasToAddNextFrameIterator;
	}

	std::vector<Camera*>::const_iterator camerasIterator = cameras_.cbegin();
	while(camerasIterator != cameras_.cend())
	{
		if (camera == *camerasIterator)
		{
			cameras_.erase(camerasIterator);
			break;
		}
		++camerasIterator;
	}
	
	if (activeCamera_ == camera)
	{
		SetActiveCamera(nullptr);
	}
}

void CameraManager::DestroyCamera(Camera* camera)
{
	RemoveCamera(camera);
	delete camera;
}

void CameraManager::SetActiveCamera(Camera* camera)
{
	activeCamera_ = camera;
	engine->GetWindowManager()->UpdateViewport();
}

bool CameraManager::DoesCameraExist(const Camera* camera)
{

	std::vector<Camera*>::const_iterator camerasIterator = cameras_.cbegin();
	while (camerasIterator != cameras_.cend())
	{
		if (camera == *camerasIterator)
		{
			return true;
		}
		++camerasIterator;
	}

	return false;
}
