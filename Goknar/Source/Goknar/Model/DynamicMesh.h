#ifndef __DYNAMICMESH_H_
#define __DYNAMICMESH_H_

#include "Mesh.h"
#include "DynamicMeshUnit.h"

class GOKNAR_API DynamicMesh : public Mesh<DynamicMeshUnit>
{
public:
	DynamicMesh();
	DynamicMesh(const Mesh& mesh);
	virtual ~DynamicMesh();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;
private:
};

#endif