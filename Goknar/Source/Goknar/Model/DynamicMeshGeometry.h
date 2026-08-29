#ifndef __DYNAMICMESHGEOMETRY_H__
#define __DYNAMICMESHGEOMETRY_H__

#include "MeshGeometry.h"

class GOKNAR_API DynamicMeshGeometry : public MeshGeometry
{
public:
	DynamicMeshGeometry();

	virtual ~DynamicMeshGeometry();

	virtual void PreInit();
	virtual void Init();
	virtual void PostInit();

	void UpdateVertexDataAt(int index, const VertexData& vertexData);

	inline int GetRendererVertexOffset() const
	{
		return rendererVertexOffset_;
	}

	inline void SetRendererVertexOffset(int rendererVertexOffset)
	{
		rendererVertexOffset_ = rendererVertexOffset;
	}

protected:

private:
	int rendererVertexOffset_{ 0 };

};

#endif