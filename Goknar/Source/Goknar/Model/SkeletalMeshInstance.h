#ifndef __SKELETALMESHINSTANCE_H__
#define __SKELETALMESHINSTANCE_H__

#include "StaticMeshInstance.h"

class GOKNAR_API SkeletalMeshInstance : public MeshInstance
{
public:
	SkeletalMeshInstance() = delete;
	SkeletalMeshInstance(Component* parentComponent);

	virtual ~SkeletalMeshInstance() {}

	virtual void Render() const override;

	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;
protected:

private:
};

#endif