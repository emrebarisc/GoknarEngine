#ifndef __STATICMESHINSTANCE_H__
#define __STATICMESHINSTANCE_H__

#include "MeshInstance.h"
#include "Model/StaticMesh.h"

class GOKNAR_API StaticMeshInstance : public MeshInstance<StaticMesh>
{
public:
	StaticMeshInstance() = delete;
	StaticMeshInstance(RenderComponent* parentComponent);

	virtual ~StaticMeshInstance() {}

	virtual void Render() override;

	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;
protected:

private:
};

#endif