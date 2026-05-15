#ifndef __CAMERACOMPONENT_H__
#define __CAMERACOMPONENT_H__

#include "Core.h"
#include "Component.h"

class Camera;

class GOKNAR_API CameraComponent : public Component
{
public:
	CameraComponent(Component* parentComponent);
	CameraComponent(const CameraComponent&) = delete;

	virtual ~CameraComponent() = default;

	virtual void Destroy() override;
	Component* Clone() const override;

	Camera* GetCamera() const
	{
		return camera_;
	}

	bool GetCameraFollowsComponentRotation() const
	{
		return cameraFollowsComponentRotation_;
	}

	void SetCameraFollowsComponentRotation(bool cameraFollowsComponent)
	{
		cameraFollowsComponentRotation_ = cameraFollowsComponent;
	}

protected:
	virtual void UpdateComponentToWorldTransformationMatrix();
	virtual void UpdateChildrenComponentToWorldTransformations();

private:
	virtual void DestroyInner() override;

	Camera* camera_{ nullptr };

	bool cameraFollowsComponentRotation_{ false };
};

#endif
