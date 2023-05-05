#ifndef __BAT_H__
#define __BAT_H__

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
	void Jump();


	AnimatedSpriteComponent* batSprite_;

	float verticalVelocity_;
};

#endif