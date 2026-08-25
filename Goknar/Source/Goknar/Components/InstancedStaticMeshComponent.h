#ifndef __INSTANCEDSTATICMESHCOMPONENT_H__
#define __INSTANCEDSTATICMESHCOMPONENT_H__

#include "MeshComponent.h"
#include "Goknar/Model/InstancedStaticMeshInstance.h"

class GOKNAR_API InstancedStaticMeshComponent : public MeshComponent<InstancedStaticMeshContainer, InstancedStaticMeshInstance>
{
public:
	InstancedStaticMeshComponent() = delete;
	InstancedStaticMeshComponent(const InstancedStaticMeshComponent&) = delete;
	InstancedStaticMeshComponent(Component* parent);
	virtual ~InstancedStaticMeshComponent();

	Component* Clone() const override;
	void SetMesh(InstancedStaticMeshContainer* mesh) override;
};

#endif
