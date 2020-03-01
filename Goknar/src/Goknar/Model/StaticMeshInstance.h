#ifndef __STATICMESHINSTANCE_H__
#define __STATICMESHINSTANCE_H__

#include "MeshInstance.h"

class GOKNAR_API StaticMeshInstance : public MeshInstance
{
public:
	StaticMeshInstance() = delete;

	StaticMeshInstance(Component* parentComponent);

	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;
	void Destroy() override;
	void SetMesh(Mesh* mesh) override;
protected:

private:
};

#endif