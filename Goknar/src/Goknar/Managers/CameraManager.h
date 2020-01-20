#ifndef __CAMERAMANAGER_H__
#define __CAMERAMANAGER_H__

#include "Goknar/Core.h"

class Camera;

class GOKNAR_API CameraManager
{
public:
	CameraManager() : activeCamera_(nullptr)
	{

	}

	~CameraManager();

	void AddCamera(Camera* camera)
	{
		cameras_.push_back(camera);

		if(cameras_.size() == 1)
		{
			activeCamera_ = camera;
		}
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