#ifndef __GAMECONTROLLER_H__
#define __GAMECONTROLLER_H__

#include "Goknar/Controller.h"

class Camera;

class PhysicsArcher;
class PhysicsArcherMovementComponent;

class PhysicsArcherGameController : public Controller
{
public:
	PhysicsArcherGameController(PhysicsArcher* archer_);
	~PhysicsArcherGameController()
	{

	}

	void BeginGame() override;

	void SetupInputs() override;

	void SetPhysicsArcher(PhysicsArcher* a)
	{
		archer_ = a;
	}

private:
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

	void ToggleDebug();

	Camera* thirdPersonCamera_{ nullptr };

	PhysicsArcher* archer_{ nullptr };
	PhysicsArcherMovementComponent* archerMovementComponent_{ nullptr };

	bool isDebugging_{ false };
	bool isInFreeCamera_{ false };
};

#endif