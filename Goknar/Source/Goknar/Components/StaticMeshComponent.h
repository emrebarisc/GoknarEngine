#ifndef __STATICMESHCOMPONENT_H__
#define __STATICMESHCOMPONENT_H__

#include "MeshComponent.h"

class StaticMesh;

class GOKNAR_API StaticMeshComponent : public MeshComponent<StaticMesh>
{
public:
	StaticMeshComponent() = delete;
	StaticMeshComponent(Component* parent);
	virtual ~StaticMeshComponent();
	
	void SetMesh(StaticMesh* mesh) override;

protected:
private:
};
#endif