#ifndef __DYNAMICICMESHCOMPONENT_H__
#define __DYNAMICICMESHCOMPONENT_H__

#include "MeshComponent.h"

class GOKNAR_API DynamicMeshComponent : public MeshComponent
{
public:
	DynamicMeshComponent() = delete;
	DynamicMeshComponent(ObjectBase* parent);
	~DynamicMeshComponent();

	void SetMesh(Mesh* mesh) override;

protected:
private:
};
#endif