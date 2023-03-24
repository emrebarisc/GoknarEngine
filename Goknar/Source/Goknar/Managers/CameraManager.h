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

	void Init();

	void AddCamera(Camera* camera);

	void SetActiveCamera(Camera* camera);

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