#ifndef __CAMERACOMPONENT_H__
#define __CAMERACOMPONENT_H__

#include "Core.h"
#include "Component.h"

class Camera;

class CameraComponent : public Component
{
public:
	CameraComponent(Component* parentComponent);

	virtual ~CameraComponent() = default;

	virtual void Destroy() override;

	Camera* GetCamera() const
	{
		return camera_;
	}

protected:
	virtual void UpdateComponentToWorldTransformationMatrix();
	virtual void UpdateChildrenComponentToWorldTransformations();

private:
	Camera* camera_{ nullptr };
};

#endif