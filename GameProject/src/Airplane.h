#ifndef __BOXGAMEOBJECT_H__
#define __BOXGAMEOBJECT_H__

#include <Goknar/ObjectBase.h>

class MeshComponent;

class Airplane : public ObjectBase
{
public:
	Airplane();
	~Airplane();

	void BeginGame() override;
	void Tick(float deltaTime) override;

private:
	void SpaceKeyDown();
	void GoLeft();
	void GoRight();

	MeshComponent* planeMeshComponent_;
	MeshComponent* planeMeshComponent2_;
	MeshComponent* planeMeshComponent3_;

	MeshComponent* propellerMeshComponent_;
	MeshComponent* propellerMeshComponent2_;
	MeshComponent* propellerMeshComponent3_;

	float moveRightSpeed_;
	float moveLefttSpeed_;
	float propellerRotationSpeed_;

};

#endif