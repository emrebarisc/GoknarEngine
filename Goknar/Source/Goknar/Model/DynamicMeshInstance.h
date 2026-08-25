#ifndef __DYNAMICMESHINSTANCE_H__
#define __DYNAMICMESHINSTANCE_H__

#include "Model/MeshContainer.h"
#include "IMeshInstance.h"

class GOKNAR_API DynamicMeshInstance : public IMeshInstance<DynamicMeshContainer>
{
public:
	DynamicMeshInstance() = delete;
	DynamicMeshInstance(RenderComponent* parentComponent);

	virtual ~DynamicMeshInstance() {}

	virtual void Render(RenderPassType renderPassType, int subMeshIndex = 0, int LODIndex = 0) override;

	void UpdateVertexDataAt(int meshIndex, int vertexIndex, const VertexData& newVertexData);

protected:
	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;

private:
};

#endif