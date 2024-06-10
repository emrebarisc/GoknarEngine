#ifndef __GAMECONTROLLER_H__
#define __GAMECONTROLLER_H__

#include "Goknar/Controller.h"
#include "Goknar/Math/InterpolatingValue.h"

class CameraComponent;

class ArcherCharacter;
class ArcherPhysicsMovementComponent;

class ArcherCharacterController : public Controller
{
public:
	ArcherCharacterController(ArcherCharacter* archer_);
	~ArcherCharacterController();

	void BeginGame() override;

	void SetupInputs() override;

	virtual void SetIsActive(bool isActive) override;

	void SetPhysicsArcher(ArcherCharacter* a)
	{
		archer_ = a;
	}

private:
	void BindInputDelegates();
	void UnbindInputDelegates();

	void OnCursorMove(double x, double y);
	void OnScrollMove(double x, double y);

	void ToggleFullscreen();
	void ToggleWindowSize();
	void ToggleFreeCamera();

	void DestroyPhysicsArcher();

	void ToggleChest();

	void DropBow();
	void EquipBow();
	void DrawBow();
	void LooseBow();

	void MoveForward();
	void StopMovingForward();
	void MoveBackward();
	void StopMovingBackward();
	void MoveLeft();
	void StopMovingLeft();
	void MoveRight();
	void StopMovingRight();

	void SetNewCameraPosition();
	void IncreaseThirdPersonCameraDistance();
	void DecreaseThirdPersonCameraDistance();

	void ToggleDebug();

	Delegate<void()> moveForwardDelegate_;
	Delegate<void()> stopMovingForwardDelegate_;
	Delegate<void()> moveBackwardDelegate_;
	Delegate<void()> stopMovingBackwardDelegate_;
	Delegate<void()> moveLeftDelegate_;
	Delegate<void()> stopMovingLeftDelegate_;
	Delegate<void()> moveRightDelegate_;
	Delegate<void()> stopMovingRightDelegate_;
	Delegate<void()> toggleDebugDelegate_;
	Delegate<void()> toggleToggleFreeCameraDelegate_;
	Delegate<void()> exitGameDelegate_;
	Delegate<void()> dropBowDelegate_;
	Delegate<void()> equipBowDelegate_;
	Delegate<void()> drawBowDelegate_;
	Delegate<void()> looseBowDelegate_;

	Delegate<void(double, double)> onScrollMoveDelegate_;
	Delegate<void(double, double)> onCursorMoveDelegate_;

	InterpolatingValue<Vector3> movementVector_;
	InterpolatingValue<float> movementRotation_;

	CameraComponent* thirdPersonCameraComponent_{ nullptr };

	ArcherCharacter* archer_{ nullptr };
	ArcherPhysicsMovementComponent* archerMovementComponent_{ nullptr };

	float thirdPersonCameraDistance_{ 1.f };
	float cameraRotationSpeed_{ 1.f };

	bool isDebugging_{ false };
	bool isInFreeCamera_{ false };
};

#endif