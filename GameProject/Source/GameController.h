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
	void IncreaseCurrentBoneIndex();
	void DecreaseCurrentBoneIndex();

	void ToggleFullscreen();

	void ToggleNormalTexture();

	void SetAnimation();
	void IncreaseAnimationIndex();
	void DecreaseAnimationIndex();

	Archer* archer;

	int currentBoneIndex;
	int animationIndex;

	unsigned int useNormalTexture : 1;
};

#endif