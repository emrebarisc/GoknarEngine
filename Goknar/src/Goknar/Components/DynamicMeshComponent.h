#ifndef __DYNAMICICMESHCOMPONENT_H__
#define __DYNAMICICMESHCOMPONENT_H__

#include "MeshComponent.h"

class DynamicMesh;

class GOKNAR_API DynamicMeshComponent : public MeshComponent
{
public:
	DynamicMeshComponent() = delete;
	DynamicMeshComponent(ObjectBase* parent);
	virtual ~DynamicMeshComponent();

	void SetMesh(Mesh* mesh) override;

	DynamicMesh* GetDynamicMesh()
	{
		return dynamicMesh_;
	}

protected:
private:
	DynamicMesh* dynamicMesh_;
};
#endif