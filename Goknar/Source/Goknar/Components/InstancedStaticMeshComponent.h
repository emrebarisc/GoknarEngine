#ifndef __INSTANCEDSTATICMESHCOMPONENT_H__
#define __INSTANCEDSTATICMESHCOMPONENT_H__

#include "MeshComponent.h"
#include "Goknar/Model/InstancedStaticMeshInstance.h"

class InstancedStaticMesh;

class GOKNAR_API InstancedStaticMeshComponent : public MeshComponent<InstancedStaticMesh, InstancedStaticMeshInstance>
{
public:
	InstancedStaticMeshComponent() = delete;
	InstancedStaticMeshComponent(const InstancedStaticMeshComponent&) = delete;
	InstancedStaticMeshComponent(Component* parent);
	virtual ~InstancedStaticMeshComponent();

	void SetMesh(InstancedStaticMesh* mesh) override;
};

#endif
