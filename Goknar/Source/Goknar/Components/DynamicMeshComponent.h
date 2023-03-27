#ifndef __DYNAMICICMESHCOMPONENT_H__
#define __DYNAMICICMESHCOMPONENT_H__

#include "MeshComponent.h"

class DynamicMesh;
class DynamicMeshInstance;

class GOKNAR_API DynamicMeshComponent : public MeshComponent<DynamicMesh, DynamicMeshInstance>
{
public:
	DynamicMeshComponent() = delete;
	DynamicMeshComponent(Component* parent);
	virtual ~DynamicMeshComponent();

	void SetMesh(DynamicMesh* mesh) override;

protected:
private:
};
#endif