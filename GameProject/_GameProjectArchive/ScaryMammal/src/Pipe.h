#ifndef __PIPE_H__
#define __PIPE_H__

#include "Goknar/ObjectBase.h"

class SpriteComponent;

class Pipe : public ObjectBase
{
public:
	Pipe();
	~Pipe();

	void BeginGame() override;
	void Tick(float deltaTime) override;

private:
	SpriteComponent* upperPipeSprite_;
	SpriteComponent* lowerPipeSprite_;

	float movementSpeed_;
};

#endif