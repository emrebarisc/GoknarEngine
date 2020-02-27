#ifndef __STATICMESHCOMPONENT_H__
#define __STATICMESHCOMPONENT_H__

#include "MeshComponent.h"

class GOKNAR_API StaticMeshComponent : public MeshComponent
{
public:
	StaticMeshComponent() = delete;
	StaticMeshComponent(ObjectBase* parent);
	~StaticMeshComponent();

protected:
private:
};
#endif