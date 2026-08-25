#ifndef __DYNAMICICMESHCOMPONENT_H__
#define __DYNAMICICMESHCOMPONENT_H__

#include "MeshComponent.h"

class DynamicMesh;
class DynamicMeshInstance;

using DynamicMeshContainer = MeshContainer<DynamicMesh>;

class GOKNAR_API DynamicMeshComponent : public MeshComponent<DynamicMeshContainer, DynamicMeshInstance>
{
public:
	DynamicMeshComponent() = delete;
	DynamicMeshComponent(const DynamicMeshComponent&) = delete;
	DynamicMeshComponent(Component* parent);
	virtual ~DynamicMeshComponent();

	Component* Clone() const override;
	void SetMesh(DynamicMeshContainer* mesh) override;

protected:
private:
};
#endif
