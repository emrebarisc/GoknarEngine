#ifndef __GAMECONTROLLER_H__
#define __GAMECONTROLLER_H__

#include "Goknar/Controller.h"

class GameController : public Controller
{
public:
	static GameController* GetInstance()
	{
		if (instance_ == nullptr)
		{
			instance_ = new GameController();
		}

		return instance_;
	}

	void SetupInputs() override;
	void HandleRestartInput();

private:
	GameController();

	~GameController()
	{

	}

	void RollCurrentObjectLeft();
	void RollCurrentObjectRight();
	void MoveCurrentObjectLeft();
	void MoveCurrentObjectRight();
	void IncreaseCurrentObjectFallSpeed();
	void DecreaseCurrentObjectFallSpeed();

	static GameController* instance_;
};

#endif