#pragma once

#include <memory>

#include "Goknar/Controller.h"
#include "Goknar/Delegates/Delegate.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Math/InterpolatingValue.h"

class DefaultCharacter;
class DefaultCharacterMovementComponent;

class SkillContainerComponent;

class DefaultCharacterController : public Controller
{
public:
	DefaultCharacterController(DefaultCharacter* character_);
	virtual ~DefaultCharacterController();

	virtual void BeginGame() override;
	virtual void Tick(float deltaTime) override;

	void SetupInputDelegates() override;
	void RemoveInputDelegates();

	void SetDefaultCharacter(DefaultCharacter* character)
	{
		character_ = character;
	}

	const Vector2i& GetCursorDeltaMoveLastFrame() const
	{
		return cursorDeltaMoveLastFrame_;
	}

private:
	void MoveForward();
	void StopMovingForward();
	void MoveBackward();
	void StopMovingBackward();
	void MoveLeft();
	void StopMovingLeft();
	void MoveRight();
	void StopMovingRight();
	void StartRunning();
	void StopRunning();
	void Jump();
	void Fire();

	void OnPauseInput();
	void OnGamePaused();
	void OnGameResumed();

	void OnCursorMove(double x, double y);

	Delegate<void()> moveForwardDelegate_;
	Delegate<void()> stopMovingForwardDelegate_;
	Delegate<void()> moveBackwardDelegate_;
	Delegate<void()> stopMovingBackwardDelegate_;
	Delegate<void()> moveLeftDelegate_;
	Delegate<void()> stopMovingLeftDelegate_;
	Delegate<void()> moveRightDelegate_;
	Delegate<void()> stopMovingRightDelegate_;
	Delegate<void()> startRunningDelegate_;
	Delegate<void()> stopRunningDelegate_;
	Delegate<void()> jumpDelegate_;
	Delegate<void()> fireDelegate_;
	
	Delegate<void()> onPauseDelegate_;
	Delegate<void(double, double)> cursorDelegate_;

	InterpolatingValue<Vector3> movementVector_;
	InterpolatingValue<float> movementRotation_;

	DefaultCharacter* character_{ nullptr };
	DefaultCharacterMovementComponent* characterMovementComponent_{ nullptr };

	Vector2i cursorDeltaMoveLastFrame_{ 0, 0 };

	bool moveForward_{ false };
	bool moveBackward_{ false };
	bool moveLeft_{ false };
	bool moveRight_{ false };
	bool onGround_{ false };
};