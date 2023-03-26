#ifndef __SKELETALMESHCOMPONENT_H__
#define __SKELETALMESHCOMPONENT_H__

#include "MeshComponent.h"

class SkeletalMesh;

class GOKNAR_API SkeletalMeshComponent : public MeshComponent
{
public:
	SkeletalMeshComponent() = delete;
	SkeletalMeshComponent(Component* parent);
	virtual ~SkeletalMeshComponent();
	
	void SetMesh(MeshUnit* mesh) override;

	SkeletalMesh* GetSkeletalMesh()
	{
		return skeletalMesh_;
	}

protected:
private:
	SkeletalMesh* skeletalMesh_;
};
#endif