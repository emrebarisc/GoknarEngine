#ifndef __DYNAMICMESHINSTANCE_H__
#define __DYNAMICMESHINSTANCE_H__

#include "Model/Mesh.h"
#include "IMeshInstance.h"

class GOKNAR_API DynamicMeshInstance : public IMeshInstance<DynamicMesh>
{
public:
	DynamicMeshInstance() = delete;
	DynamicMeshInstance(RenderComponent* parentComponent);

	virtual ~DynamicMeshInstance() {}

<<<<<<< HEAD
	virtual void Render(RenderPassType renderPassType, int subMeshIndex = 0, int LODIndex = 0) override;
=======
	virtual void Render(int subMeshIndex, RenderPassType renderPassType = RenderPassType::Forward) override;
>>>>>>> master

	void UpdateVertexDataAt(int meshIndex, int vertexIndex, const VertexData& newVertexData);

protected:
	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;

private:
};

#endif