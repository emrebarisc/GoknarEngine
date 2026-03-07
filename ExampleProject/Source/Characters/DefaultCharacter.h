#pragma once 

#include "BaseCharacter.h"

#include "Goknar/Math/InterpolatingValue.h"

class AnimationGraph;
class CameraComponent;
class SkeletalMesh;

class SocketComponent;
class DefaultCharacterController;
class DefaultCharacterMovementComponent;
class Weapon;

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
	virtual void Idle() override;
	virtual void Jump() override;
	virtual void Fire();
	virtual void ToggleCrouch();
	virtual void StartRunning();
	virtual void StopRunning();

	void SetIsStrafing(bool isStrafing);
	bool GetIsStrafing() const
	{
		return isStrafing_;
	}

protected:
	std::string GetStandAnimation(const Vector3& dir, bool isRunning);
	std::string GetCrouchAnimation(const Vector3& dir);
	std::string GetStrafingStandAnimation(const Vector3& dir, bool isRunning);
	std::string GetStrafingCrouchAnimation(const Vector3& dir);

private:
	void SetupAnimationGraph();

	Weapon* weapon_{ nullptr };

	SkeletalMesh* skeletalMesh_{ nullptr };

	AnimationGraph* animationGraph_{ nullptr };

	CameraComponent* thirdPersonCameraComponent_{ nullptr };
	DefaultCharacterMovementComponent* defaultCharacterMovementComponent_{ nullptr };

	InterpolatingValue<float> cameraDistance_;
	InterpolatingValue<Vector3> cameraHeightOffset_;

	float cameraYaw_{ 0.f };
	float cameraPitch_{ 0.f };
	float defaultCameraDistance_{ 6.f };
	float strafingCameraDistance_{ 2.f };
	
	float mouseSensitivity_{ 1.f };
	float minPitch_{ -40.f };
	float maxPitch_{ 40.f };

	float timeSinceStopped_{ 0.f };

	bool isCrouched_{ false };
	bool isStrafing_{ false };
};