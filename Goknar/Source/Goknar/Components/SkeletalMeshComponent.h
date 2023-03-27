#ifndef __SKELETALMESHCOMPONENT_H__
#define __SKELETALMESHCOMPONENT_H__

#include "MeshComponent.h"

class SkeletalMesh;

class GOKNAR_API SkeletalMeshComponent : public MeshComponent<SkeletalMesh>
{
public:
	SkeletalMeshComponent() = delete;
	SkeletalMeshComponent(Component* parent);
	virtual ~SkeletalMeshComponent();
	
	void SetMesh(SkeletalMesh* mesh) override;
protected:
private:
};
#endif