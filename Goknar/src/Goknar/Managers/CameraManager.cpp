#include "pch.h"

#include "CameraManager.h"

CameraManager::~CameraManager()
{
	for (Camera* camera : cameras_)
	{
		delete camera;
	}
}
