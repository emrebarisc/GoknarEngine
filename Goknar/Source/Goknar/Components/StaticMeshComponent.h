#ifndef __STATICMESHCOMPONENT_H__
#define __STATICMESHCOMPONENT_H__

#include "MeshComponent.h"

class StaticMesh;

class GOKNAR_API StaticMeshComponent : public MeshComponent
{
public:
	StaticMeshComponent() = delete;
	StaticMeshComponent(ObjectBase* parent);
	virtual ~StaticMeshComponent();
	
	void SetMesh(MeshUnit* mesh) override;

	StaticMesh* GetStaticMesh()
	{
		return staticMesh_;
	}

protected:
private:
	StaticMesh* staticMesh_;
};
#endif