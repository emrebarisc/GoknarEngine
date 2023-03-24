#ifndef __GAMECONTROLLER_H__
#define __GAMECONTROLLER_H__

#include "Goknar/Controller.h"

class Archer;

class GameController : public Controller
{
public:
	GameController();
	~GameController()
	{

	}

	void SetupInputs() override;

	void SetArcher(Archer* a)
	{
		archer = a;
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

	Archer* archer;
};

#endif