#ifndef __GAMECONTROLLER_H__
#define __GAMECONTROLLER_H__

#include "Goknar/Controller.h"
#include "Goknar/Math/InterpolatingValue.h"

class Camera;

class ArcherCharacter;
class ArcherCharacterMovementComponent;

class ArcherCharacterController : public Controller
{
public:
	ArcherCharacterController(ArcherCharacter* archer_);
	~ArcherCharacterController()
	{

	}

	void BeginGame() override;

	void SetupInputs() override;

	void SetPhysicsArcher(ArcherCharacter* a)
	{
		archer_ = a;
	}

private:
	void OnCursorMove(double x, double y);
	void OnScrollMove(double x, double y);

	void ToggleFullscreen();
	void ToggleWindowSize();
	void ToggleFreeCamera();

	void ExitGame();
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

	void ToggleDebug();


	InterpolatingValue<Vector3> movementVector_;
	InterpolatingValue<float> movementRotation_;

	Camera* thirdPersonCamera_{ nullptr };

	ArcherCharacter* archer_{ nullptr };
	ArcherCharacterMovementComponent* archerMovementComponent_{ nullptr };

	bool isDebugging_{ false };
	bool isInFreeCamera_{ false };
};

#endif