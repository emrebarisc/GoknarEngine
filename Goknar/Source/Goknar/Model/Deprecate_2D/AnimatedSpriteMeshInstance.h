#ifndef __ANIMATEDSPRITEMESHINSTANCE_H__
#define __ANIMATEDSPRITEMESHINSTANCE_H__

#include "Goknar/Model/DynamicMeshInstance.h"

class GOKNAR_API AnimatedSpriteMeshInstance : public DynamicMeshInstance
{
public:
	AnimatedSpriteMeshInstance() = delete;
	AnimatedSpriteMeshInstance(RenderComponent* parentComponent);

	virtual ~AnimatedSpriteMeshInstance() {}

private:
};

#endif