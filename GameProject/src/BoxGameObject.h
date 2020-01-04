#ifndef __BOXGAMEOBJECT_H__
#define __BOXGAMEOBJECT_H__

#include <Goknar/ObjectBase.h>

class MeshComponent;

class BoxGameObject : public ObjectBase
{
public:
	BoxGameObject();
	~BoxGameObject();

	void BeginGame() override;
	void Tick(float deltaTime) override;

private:
	void SpaceKeyDown();

	MeshComponent* boxMeshComponent_;

};

#endif