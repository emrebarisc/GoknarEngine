#ifndef __STATICMESH_H__
#define __STATICMESH_H__

#include "MeshUnit.h"

class GOKNAR_API StaticMesh : public MeshUnit
{
public:
	StaticMesh();

	virtual ~StaticMesh();

	virtual void Init();

private:
};

#endif