#ifndef __INSTANCEDSTATICMESHINSTANCE_H__
#define __INSTANCEDSTATICMESHINSTANCE_H__

#include "IMeshInstance.h"
#include "Model/Mesh.h"

class GOKNAR_API InstancedStaticMeshInstance : public IMeshInstance<InstancedStaticMesh>
{
public:
	InstancedStaticMeshInstance() = delete;
	InstancedStaticMeshInstance(RenderComponent* parentComponent);

	virtual ~InstancedStaticMeshInstance() {}

	virtual void Render(RenderPassType renderPassType, int subMeshIndex = 0, int LODIndex = 0) override;

	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;
};

#endif
