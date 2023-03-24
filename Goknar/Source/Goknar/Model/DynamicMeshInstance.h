#ifndef __DYNAMICMESHINSTANCE_H__
#define __DYNAMICMESHINSTANCE_H__

#include "Model/MeshUnit.h"
#include "MeshInstance.h"

class GOKNAR_API DynamicMeshInstance : public MeshInstance
{
public:
	DynamicMeshInstance() = delete;
	DynamicMeshInstance(RenderComponent* parentComponent);

	virtual ~DynamicMeshInstance() {}

	void UpdateVertexDataAt(int index, const VertexData& newVertexData);

protected:
	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;

private:
};

#endif