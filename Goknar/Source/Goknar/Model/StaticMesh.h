#ifndef __STATICMESH_H__
#define __STATICMESH_H__

#include "MeshUnit.h"

class GOKNAR_API StaticMesh : public MeshUnit
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