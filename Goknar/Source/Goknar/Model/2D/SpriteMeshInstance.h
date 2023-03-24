#ifndef __SPRITEMESHINSTANCE_H__
#define __SPRITEMESHINSTANCE_H__

#include "Goknar/Model/DynamicMeshInstance.h"

class GOKNAR_API SpriteMeshInstance : public DynamicMeshInstance
{
public:
	SpriteMeshInstance() = delete;
	SpriteMeshInstance(RenderComponent* parentComponent);
private:
};

#endif