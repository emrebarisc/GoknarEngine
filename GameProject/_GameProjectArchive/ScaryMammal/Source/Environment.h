#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include "Goknar/ObjectBase.h"

class AnimatedSpriteComponent;
class SpriteComponent;
class Pipe;

class Environment : public ObjectBase
{
public:
	Environment();
	~Environment();

	void BeginGame();
	void Tick(float deltaTime);

protected:

private:
	SpriteComponent* background_;
	SpriteComponent* floor_;

	AnimatedSpriteComponent* firedSkulls_;

	Pipe* pipe1_;
	Pipe* pipe2_;
	Pipe* pipe3_;

};

#endif