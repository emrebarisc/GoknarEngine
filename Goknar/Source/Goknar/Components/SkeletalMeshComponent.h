#ifndef __SKELETALMESHCOMPONENT_H__
#define __SKELETALMESHCOMPONENT_H__

#include "MeshComponent.h"

class SkeletalMesh;
class SkeletalMeshInstance;

class GOKNAR_API SkeletalMeshComponent : public MeshComponent<SkeletalMesh, SkeletalMeshInstance>
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