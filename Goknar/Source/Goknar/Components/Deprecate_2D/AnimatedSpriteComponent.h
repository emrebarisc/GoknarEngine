#ifndef __ANIMATEDSPRITECOMPONENT_H__
#define __ANIMATEDSPRITECOMPONENT_H__

#include "DynamicMeshComponent.h"

class AnimatedSpriteMesh;

class GOKNAR_API AnimatedSpriteComponent : public DynamicMeshComponent
{
public:
	AnimatedSpriteComponent() = delete;
	AnimatedSpriteComponent(Component* parent);
	virtual ~AnimatedSpriteComponent();

	void SetMesh(DynamicMesh* mesh) override;
protected:

private:
};
#endif