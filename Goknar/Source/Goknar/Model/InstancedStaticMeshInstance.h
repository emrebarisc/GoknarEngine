#ifndef __INSTANCEDSTATICMESHINSTANCE_H__
#define __INSTANCEDSTATICMESHINSTANCE_H__

#include "IMeshInstance.h"
#include "Model/InstancedStaticMesh.h"

class GOKNAR_API InstancedStaticMeshInstance : public IMeshInstance<InstancedStaticMesh>
{
public:
	InstancedStaticMeshInstance() = delete;
	InstancedStaticMeshInstance(RenderComponent* parentComponent);

	virtual ~InstancedStaticMeshInstance() {}

	virtual void Render(int subMeshIndex, RenderPassType renderPassType = RenderPassType::Forward) override;

	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;
};

#endif
