#ifndef __STATICMESHINSTANCE_H__
#define __STATICMESHINSTANCE_H__

#include "IMeshInstance.h"
#include "Model/StaticMesh.h"

class GOKNAR_API StaticMeshInstance : public IMeshInstance<StaticMesh>
{
public:
	StaticMeshInstance() = delete;
	StaticMeshInstance(RenderComponent* parentComponent);

	virtual ~StaticMeshInstance() {}

	virtual void Render(RenderPassType renderPassType = RenderPassType::Forward) override;

	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;
protected:

private:
};

#endif