#ifndef __DYNAMICMESHINSTANCE_H__
#define __DYNAMICMESHINSTANCE_H__

#include "Mesh.h"
#include "MeshInstance.h"

class GOKNAR_API DynamicMeshInstance : public MeshInstance
{
public:
	DynamicMeshInstance() = delete;
	DynamicMeshInstance(Component* parentComponent);

	virtual ~DynamicMeshInstance() {}

	void UpdateVertexDataAt(int index, const VertexData& newVertexData);

protected:
	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;
	void Destroy() override; 
	void SetMesh(Mesh* mesh) override;

private:
};

#endif