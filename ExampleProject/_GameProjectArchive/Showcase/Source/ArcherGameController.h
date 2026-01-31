#ifndef __GAMECONTROLLER_H__
#define __GAMECONTROLLER_H__

#include "Goknar/Controller.h"
#include "Delegates/Delegate.h"

class Camera;

class Archer;
class ArcherMovementComponent;

class ArcherGameController : public Controller
{
public:
	ArcherGameController(Archer* archer_);
	~ArcherGameController();

	void BeginGame() override;

	void SetupInputs() override;

	void SetArcher(Archer* a)
	{
		archer_ = a;
	}

private:
	void OnCursorMove(double x, double y);
	void OnScrollMove(double x, double y);

	void ToggleFullscreen();
	void ToggleWindowSize();

	void DestroyArcher();

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

	Delegate<void()> moveForwardDelegate_;
	Delegate<void()> stopMovingForwardDelegate_;
	Delegate<void()> moveBackwardDelegate_;
	Delegate<void()> stopMovingBackwardDelegate_;
	Delegate<void()> moveLeftDelegate_;
	Delegate<void()> stopMovingLeftDelegate_;
	Delegate<void()> moveRightDelegate_;
	Delegate<void()> stopMovingRightDelegate_;

	Delegate<void(double, double)> onScrollMoveDelegate_;
	Delegate<void(double, double)> onCursorMoveDelegate_;

	Camera* thirdPersonCamera_{ nullptr };

	Archer* archer_{ nullptr };
	ArcherMovementComponent* archerMovementComponent_{ nullptr };

	bool isDebugging_{ false };
};

#endif