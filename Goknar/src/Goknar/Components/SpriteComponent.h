#ifndef __SPRITECOMPONENT_H__
#define __SPRITECOMPONENT_H__

#include "MeshComponent.h"

#include "Goknar/Core.h"

class SpriteMesh;
class MeshInstance;
class ObjectBase;

class GOKNAR_API SpriteComponent : public MeshComponent
{
public:
	SpriteComponent() = delete;
	SpriteComponent(ObjectBase* parent);
	~SpriteComponent();

	void SetMesh(Mesh* sprite) override;

	SpriteMesh* GetSpriteMesh()
	{
		return spriteMesh_;
	}

protected:

private:
	SpriteMesh* spriteMesh_;
};
#endif