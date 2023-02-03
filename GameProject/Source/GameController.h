#ifndef __GAMECONTROLLER_H__
#define __GAMECONTROLLER_H__

#include "Goknar/Controller.h"

class Mutant;

class GameController : public Controller
{
public:
	GameController();
	~GameController()
	{

	}

	void SetupInputs() override;

	void SetMutant(Mutant* m)
	{
		mutant = m;
	}

private:
	void IncreaseCurrentBoneIndex();
	void DecreaseCurrentBoneIndex();

	void ToggleNormalTexture();

	void SetAnimation();
	void IncreaseAnimationIndex();
	void DecreaseAnimationIndex();

	Mutant* mutant;

	int currentBoneIndex;
	int animationIndex;

	unsigned int useNormalTexture : 1;
};

#endif