#ifndef __HYENA_H__
#define __HYENA_H__

#include "Goknar/ObjectBase.h"

class AnimatedSpriteComponent;

class Hyena : public ObjectBase
{
public:
	Hyena();
	~Hyena();

	void BeginGame() override;
	void Tick(float deltaTime) override;

private:
	void WalkForward();
	void WalkForwardStopped();
	void WalkBackward();
	void WalkBackwardStopped();
	void WalkLeftward();
	void WalkLeftwardStopped();
	void WalkRightward();
	void WalkRightwardStopped();

	void DetermineAnimation();

	Vector3 movementDirection_;

	AnimatedSpriteComponent* hyenaSprite_;

	float velocity_;
};

#endif