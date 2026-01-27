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
	int currentCamerasToAddNextFrameCount = camerasToAddNextFrameCount_;

	for (int cameraToAddNextFrameIndex = 0; cameraToAddNextFrameIndex < currentCamerasToAddNextFrameCount; ++cameraToAddNextFrameIndex)
	{
		Camera* currentCamera = camerasToAddNextFrame_[cameraToAddNextFrameIndex];

		if (camera == currentCamera)
		{
			std::vector<Camera*>::const_iterator iterator = camerasToAddNextFrame_.cbegin();
			std::advance(iterator, cameraToAddNextFrameIndex);

			camerasToAddNextFrame_.erase(iterator);
			--camerasToAddNextFrameCount_;
			--cameraToAddNextFrameIndex;
			--currentCamerasToAddNextFrameCount;
		}
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
