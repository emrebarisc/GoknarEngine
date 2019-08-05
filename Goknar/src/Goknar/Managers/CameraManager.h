#ifndef __CAMERAMANAGER_H__
#define __CAMERAMANAGER_H__

#include "Goknar/Core.h"

class Camera;

class GOKNAR_API CameraManager
{
public:
	CameraManager()
	{

	}

	~CameraManager()
	{
		for (Camera* camera : cameras_)
		{
			delete camera;
		}
	}

	void AddCamera(Camera* camera)
	{
		cameras_.push_back(camera);

		// TEMPORARY
		activeCamera_ = camera;
	}

	void SetActiveCamera(Camera* camera)
	{
		activeCamera_ = camera;
	}

	Camera* GetActiveCamera() const
	{
		return activeCamera_;
	}

protected:

private:
	Camera* activeCamera_;

	std::vector<Camera*> cameras_;
};

#endif