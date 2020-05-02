#ifndef __TESTSPRITE_H__
#define __TESTSPRITE_H__

#include "Goknar/ObjectBase.h"

class AnimatedSpriteComponent;

class Bat : public ObjectBase
{
public:
	Bat();
	~Bat();

	void BeginGame() override;
	void Tick(float deltaTime) override;

private:
	AnimatedSpriteComponent* batSprite_;
};

#endif