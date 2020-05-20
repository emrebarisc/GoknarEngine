#ifndef __DECEASED_H__
#define __DECEASED_H__

#include "Goknar/ObjectBase.h"

class AnimatedSpriteComponent;

class Deceased : public ObjectBase
{
public:
	Deceased();
	~Deceased();

	void BeginGame() override;
	void Tick(float deltaTime) override;

	void Hurt(float damage);

	bool GetIsDead() const
	{
		return health_ <= 0;
	}

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

	AnimatedSpriteComponent* deceasedSprite_;

	float velocity_;
	float health_;
};

#endif