#ifndef __DYNAMICMESH_H_
#define __DYNAMICMESH_H_

#include "Mesh.h"

class GOKNAR_API DynamicMesh : public Mesh
{
public:
	DynamicMesh();
	DynamicMesh(const Mesh& mesh);

	~DynamicMesh();

	virtual void Init();

	void UpdateVertexDataAt(int index, const VertexData& vertexData);

	int GetRendererVertexOffset() const
	{
		return rendererVertexOffset_;
	}

	void SetRendererVertexOffset(int rendererVertexOffset)
	{
		rendererVertexOffset_ = rendererVertexOffset;
	}

private:
	int rendererVertexOffset_;
};

#endif