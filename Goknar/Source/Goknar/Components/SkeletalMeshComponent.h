#ifndef __SKELETALMESHCOMPONENT_H__
#define __SKELETALMESHCOMPONENT_H__

#include "MeshComponent.h"

class SkeletalMesh;
class SkeletalMeshInstance;

using SkeletalMeshContainer = MeshContainer<SkeletalMesh>;

class GOKNAR_API SkeletalMeshComponent : public MeshComponent<SkeletalMeshContainer, SkeletalMeshInstance>
{
public:
	SkeletalMeshComponent() = delete;
	SkeletalMeshComponent(const SkeletalMeshComponent&) = delete;
	SkeletalMeshComponent(Component* parent);
	virtual ~SkeletalMeshComponent();
	
	Component* Clone() const override;
	void SetMesh(SkeletalMeshContainer* mesh) override;
protected:
private:
};
#endif
