#ifndef __DYNAMICMESH_H__
#define __DYNAMICMESH_H__

#include "Mesh.h"
<<<<<<< HEAD

#endif
=======
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
>>>>>>> master
