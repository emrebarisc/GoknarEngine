#ifndef __STATICMESH_H__
#define __STATICMESH_H__

#include "Mesh.h"

class GOKNAR_API StaticMesh : public Mesh
{
public:
	StaticMesh();

	virtual ~StaticMesh();

	virtual void Init();

private:
};

#endif