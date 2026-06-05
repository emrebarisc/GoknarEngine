#pragma once 

#include <memory>

#include "BaseCharacter.h"

#include "Goknar/Math/InterpolatingValue.h"

struct AnimationState;
struct AnimationGraph;
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

private:
	Weapon* weapon_{ nullptr };

	SkeletalMesh* skeletalMesh_{ nullptr };

	AnimationGraph* animationGraph_{ nullptr };

	CameraComponent* thirdPersonCameraComponent_{ nullptr };
	DefaultCharacterMovementComponent* defaultCharacterMovementComponent_{ nullptr };

	InterpolatingValue<float> cameraDistance_;
	InterpolatingValue<Vector3> cameraHeightOffset_;
	InterpolatingValue<float> cameraShoulderOffset_;

	std::shared_ptr<AnimationState> standingState_;
	std::shared_ptr<AnimationState> crouchState_;
	std::shared_ptr<AnimationState> standingStrafingState_;
	std::shared_ptr<AnimationState> crouchStrafingState_;

	float cameraYaw_{ 0.f };
	float cameraPitch_{ 0.f };
	float defaultCameraDistance_{ 6.f };
	float strafingCameraDistance_{ 2.25f };
	float defaultShoulderOffset_{ 0.35f };
	float strafingShoulderOffset_{ 0.55f };
	float cameraCollisionPadding_{ 0.2f };
	float minCameraDistance_{ 0.65f };
	float minCameraForwardZ_{ -0.95f };
	float maxCameraForwardZ_{ 0.75f };
	
	float mouseSensitivity_{ 1.f };

	float timeSinceStopped_{ 0.f };

	bool isCrouched_{ false };
	bool isStrafing_{ false };
};