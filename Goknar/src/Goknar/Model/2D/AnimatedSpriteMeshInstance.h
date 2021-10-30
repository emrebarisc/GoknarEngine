#ifndef __ANIMATEDSPRITEMESHINSTANCE_H__
#define __ANIMATEDSPRITEMESHINSTANCE_H__

#include "Goknar/Model/DynamicMeshInstance.h"

class AnimatedSpriteMesh;

class GOKNAR_API AnimatedSpriteMeshInstance : public DynamicMeshInstance
{
public:
	AnimatedSpriteMeshInstance() = delete;
	AnimatedSpriteMeshInstance(Component* parentComponent);

	virtual ~AnimatedSpriteMeshInstance() {}

	void SetAnimatedSpriteMesh(AnimatedSpriteMesh* animatedspriteMesh);
	AnimatedSpriteMesh* GetAnimatedSpriteMesh()
	{
		return animatedspriteMesh_;
	}

private:
	AnimatedSpriteMesh* animatedspriteMesh_;
};

#endif