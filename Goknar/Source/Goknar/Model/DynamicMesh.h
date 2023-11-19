#ifndef __DYNAMICMESH_H_
#define __DYNAMICMESH_H_

#include "MeshUnit.h"

class GOKNAR_API DynamicMesh : public MeshUnit
{
public:
	DynamicMesh();
	DynamicMesh(const MeshUnit& mesh);
	virtual ~DynamicMesh();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

	void UpdateVertexDataAt(int index, const VertexData& vertexData);

	inline int GetRendererVertexOffset() const
	{
		return rendererVertexOffset_;
	}

	inline void SetRendererVertexOffset(int rendererVertexOffset)
	{
		rendererVertexOffset_ = rendererVertexOffset;
	}

private:
	int rendererVertexOffset_{ 0 };
};

#endif