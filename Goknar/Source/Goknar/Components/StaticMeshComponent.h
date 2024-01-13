#ifndef __STATICMESHCOMPONENT_H__
#define __STATICMESHCOMPONENT_H__

#include "MeshComponent.h"

class StaticMesh;
class StaticMeshInstance;

class GOKNAR_API StaticMeshComponent : public MeshComponent<StaticMesh, StaticMeshInstance>
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