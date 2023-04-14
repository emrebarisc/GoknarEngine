#ifndef __GAMECONTROLLER_H__
#define __GAMECONTROLLER_H__

#include "Goknar/Controller.h"

class Camera;

class Archer;
class ArcherMovementComponent;

class ArcherGameController : public Controller
{
public:
	ArcherGameController(Archer* archer_);
	~ArcherGameController()
	{

	}

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

	Camera* thirdPersonCamera_{ nullptr };

	Archer* archer_{ nullptr };
	ArcherMovementComponent* archerMovementComponent_{ nullptr };
};

#endif