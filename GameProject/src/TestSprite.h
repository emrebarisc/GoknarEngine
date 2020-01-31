#ifndef __TESTSPRITE_H__
#define __TESTSPRITE_H__

#include "Goknar/ObjectBase.h"

class MeshComponent;

class TestSprite : public ObjectBase
{
public:
	TestSprite();
	~TestSprite();

	void BeginGame() override;
	void Tick(float deltaTime) override;

private:
	MeshComponent* spriteMeshComponent_;
};

#endif