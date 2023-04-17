#ifndef __SKELETALMESHINSTANCE_H__
#define __SKELETALMESHINSTANCE_H__

#include "MeshInstance.h"

#include <unordered_map>

#include "Model/SkeletalMesh.h"

class SkeletalAnimation;
class SocketComponent;

struct SkeletalMeshAnimation
{
	std::string name{ "" };
	const SkeletalAnimation* skeletalAnimation{ nullptr };
	std::function<void()> animationDoneCallback;
	float animationTime{ 0.f };
	float elapsedTimeInSeconds{ 0.f };
	float initialTimeInSeconds{ 0.f };
	bool playOnce{ false };
};

class GOKNAR_API SkeletalMeshInstance : public MeshInstance<SkeletalMesh>
{
public:
	SkeletalMeshInstance() = delete;
	SkeletalMeshInstance(RenderComponent* parentComponent);
	virtual ~SkeletalMeshInstance();
	virtual void Render() override;

	virtual void SetMesh(SkeletalMesh* skeletalMesh) override;

	void PlayAnimation(const std::string& animationName, bool playAnimationOnce = false, const std::function<void()>& callback = {});

	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;

	SocketComponent* AddSocketToBone(const std::string& boneName);
	SocketComponent* GetSocket(const std::string& boneName);
protected:

private:
	SkeletalMeshAnimation skeletalMeshAnimation_;

	std::vector<Matrix> boneTransformations_;
	std::unordered_map<std::string, SocketComponent*> sockets_;
};

#endif