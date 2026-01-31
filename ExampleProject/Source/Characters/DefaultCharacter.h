#pragma once 

#include "BaseCharacter.h"

#include "Goknar/Math/InterpolatingValue.h"

class CameraComponent;
class SkeletalMesh;

class SocketComponent;
class DefaultCharacterController;

class GOKNAR_API DefaultCharacter : public BaseCharacter
{
public:
	DefaultCharacter();
	virtual ~DefaultCharacter();
	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;
	
	CameraComponent* GetThirdPersonCameraComponent() const
	{
		return thirdPersonCameraComponent_;
	}

	virtual void Die() override;

private:
	SkeletalMesh* skeletalMesh_{ nullptr };

	CameraComponent* thirdPersonCameraComponent_{ nullptr };

	InterpolatingValue<float> cameraDistance_;

	float cameraYaw_ = 0.0f;
	float cameraPitch_ = 0.0f;
	float defaultCameraDistance_ = 8.0f;
	
	float mouseSensitivity_ = 1.f;
	float minPitch_ = -40.0f;
	float maxPitch_ = 40.0f;
};