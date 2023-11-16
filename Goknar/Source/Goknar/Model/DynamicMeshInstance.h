#ifndef __DYNAMICMESHINSTANCE_H__
#define __DYNAMICMESHINSTANCE_H__

#include "Model/DynamicMesh.h"
#include "MeshInstance.h"

class GOKNAR_API DynamicMeshInstance : public MeshInstance<DynamicMesh>
{
public:
	DynamicMeshInstance() = delete;
	DynamicMeshInstance(RenderComponent* parentComponent);

	virtual ~DynamicMeshInstance() {}

	virtual void Render(RenderPassType renderPassType = RenderPassType::Forward) override;

	void UpdateVertexDataAt(int index, const VertexData& newVertexData);

protected:
	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;

private:
};

#endif