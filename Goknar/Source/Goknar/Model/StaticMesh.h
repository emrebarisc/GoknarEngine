#ifndef __STATICMESH_H__
#define __STATICMESH_H__

#include "Mesh.h"
#include "MeshUnit.h"

class GOKNAR_API StaticMesh : public Mesh<MeshUnit>
{
public:
	StaticMesh();

	virtual ~StaticMesh();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

private:
};

#endif