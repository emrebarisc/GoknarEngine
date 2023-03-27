#ifndef __DYNAMICICMESHCOMPONENT_H__
#define __DYNAMICICMESHCOMPONENT_H__

#include "MeshComponent.h"

class DynamicMesh;

class GOKNAR_API DynamicMeshComponent : public MeshComponent<DynamicMesh>
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