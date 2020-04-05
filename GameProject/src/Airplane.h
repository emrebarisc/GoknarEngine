#ifndef __BOXGAMEOBJECT_H__
#define __BOXGAMEOBJECT_H__

#include <Goknar/ObjectBase.h>

class StaticMeshComponent;

class Airplane : public ObjectBase
{
public:
	Airplane();
	~Airplane();

	void BeginGame() override;
	void Tick(float deltaTime) override;

private:
	void SpaceKeyDown();
	void PositiveYaw();
	void NegativeYaw();
	void PositivePitch();
	void NegativePitch();

	StaticMeshComponent* planeMeshComponent_;
	StaticMeshComponent* propellerMeshComponent_;

	float moveRightSpeed_;
	float moveLefttSpeed_;
	float propellerRotationSpeed_;
};

#endif