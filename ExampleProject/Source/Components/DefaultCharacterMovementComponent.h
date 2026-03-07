#pragma once

#include "Goknar/Physics/Components/PhysicsMovementComponent.h"
#include "Goknar/Math/InterpolatingValue.h"

class Camera;
class CameraComponent;
class DefaultCharacter;

class GOKNAR_API DefaultCharacterMovementComponent : public PhysicsMovementComponent
{
public:
	DefaultCharacterMovementComponent(Component* parent);
	~DefaultCharacterMovementComponent()
	{

	}

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	virtual void BeginGame() override;
	virtual void TickComponent(float deltaTime) override;

	virtual void AddMovementDirection(const Vector3& movementDirection);

	void StartRunning();
	void StartWalking();
	void StartCrouching();
	virtual void Jump(const Vector3& v = Vector3::ZeroVector) override;

	void SetIsStrafing(bool isStrafing)
	{
		isStrafing_ = isStrafing;
	}

	inline bool GetIsRunning() const
	{
		return (WALK_SPEED * 1.25f) < GetLinearVelocity().Length();
	}

	inline static constexpr float MIN_SPEED = 0.01f;
	inline static constexpr float CROUCH_SPEED = 0.025f;
	inline static constexpr float WALK_SPEED = 0.05f;
	inline static constexpr float RUN_SPEED = 0.1f;

private:
	void OnMovementDirectionInterpolated();
	void OnMovementRotationInterpolated();

	InterpolatingValue<Vector3> movementDirection_;
	InterpolatingValue<float> movementRotation_;

	DefaultCharacter* ownerCharacter_{ nullptr };
	Camera* thirdPersonCamera_{ nullptr };
	CameraComponent* thirdPersonCameraComponent_{ nullptr };

	bool isStrafing_ = false;
};