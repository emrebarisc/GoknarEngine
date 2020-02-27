#ifndef __DYNAMICMESHINSTANCE_H__
#define __DYNAMICMESHINSTANCE_H__

#include "MeshInstance.h"

class GOKNAR_API DynamicMeshInstance : public MeshInstance
{
public:
	DynamicMeshInstance() = delete;

	DynamicMeshInstance(Component* parentComponent);

protected:
	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;
	void Destroy() override;

private:
};

#endif