#ifndef __DYNAMICMESHUNIT_H__
#define __DYNAMICMESHUNIT_H__

#include "MeshUnit.h"

class GOKNAR_API DynamicMeshUnit : public MeshUnit
{
public:
	DynamicMeshUnit();

	virtual ~DynamicMeshUnit();

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