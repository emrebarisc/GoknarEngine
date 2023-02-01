#ifndef __SKELETALMESHINSTANCE_H__
#define __SKELETALMESHINSTANCE_H__

#include "StaticMeshInstance.h"

class SkeletalAnimation;

struct SkeletalMeshAnimation
{
	std::string name = "";
	const SkeletalAnimation* skeletalAnimation = nullptr;
	float animationTime = 0.f;
	float elapsedTimeInSeconds = 0.f;
};

class GOKNAR_API SkeletalMeshInstance : public MeshInstance
{
public:
	SkeletalMeshInstance() = delete;
	SkeletalMeshInstance(Component* parentComponent);
	virtual ~SkeletalMeshInstance() {}
	virtual void Render() override;

	void PlayAnimation(const std::string& animationName);

	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;
protected:

private:
	SkeletalMeshAnimation skeletalMeshAnimation_;
};

#endif