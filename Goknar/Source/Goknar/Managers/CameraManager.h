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

	void PreInit();
	void Init();
	void PostInit();

	void AddCamera(Camera* camera);
	void RemoveCamera(Camera* camera);
	void DestroyCamera(Camera* camera);
	
	const std::vector<Camera*>& GetCameras() const
	{
		return cameras_;
	}

	void HandleNewlyAddedCameras();

	void SetActiveCamera(Camera* camera);

	Camera* GetActiveCamera() const
	{
		return activeCamera_;
	}

	bool DoesCameraExist(const Camera* camera);

protected:

private:
	Camera* activeCamera_;

	std::vector<Camera*> cameras_{};
	std::vector<Camera*> camerasToAddNextFrame_{};

	int camerasToAddNextFrameCount_;
};

#endif