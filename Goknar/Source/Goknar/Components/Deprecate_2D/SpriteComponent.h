#ifndef __SPRITECOMPONENT_H__
#define __SPRITECOMPONENT_H__

#include "MeshComponent.h"

#include "Goknar/Core.h"

class SpriteMesh;
class ObjectBase;

template<class T>
class GOKNAR_API SpriteComponent : public MeshComponent<SpriteMesh>
{
public:
	SpriteComponent() = delete;
	SpriteComponent(Component* parent);
	virtual ~SpriteComponent();

	void SetMesh(SpriteMesh* sprite) override;
protected:

private:
};
#endif