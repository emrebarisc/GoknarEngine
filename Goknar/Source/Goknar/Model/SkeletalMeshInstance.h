#ifndef __SKELETALMESHINSTANCE_H__
#define __SKELETALMESHINSTANCE_H__

#include "MeshInstance.h"

#include <unordered_map>

#include "Model/SkeletalMesh.h"

class SkeletalAnimation;
class SocketComponent;

struct GOKNAR_API PlayLoopData
{
	bool playOnce{ false };
	std::function<void()> callback;
};

struct GOKNAR_API KeyframeData
{
	void AddCallbackToKeyframe(int keyframe, const std::function<void()>& callback)
	{
		keyframeCallbackMap.insert({ keyframe, callback });
	}

	std::unordered_map<int, std::function<void()>> keyframeCallbackMap;
};

struct GOKNAR_API SkeletalMeshAnimation
{
	std::string name{ "" };
	PlayLoopData playLoopData;
	KeyframeData keyframeData;
	const SkeletalAnimation* skeletalAnimation{ nullptr };
	float animationTime{ 0.f };
	float elapsedTimeInSeconds{ 0.f };
	float initialTimeInSeconds{ 0.f };
	int currentKeyframe{ 0 };
};

class GOKNAR_API SkeletalMeshInstance : public MeshInstance<SkeletalMesh>
{
public:
	SkeletalMeshInstance() = delete;
	SkeletalMeshInstance(RenderComponent* parentComponent);
	virtual ~SkeletalMeshInstance();
	virtual void Render(RenderPassType renderPassType = RenderPassType::Main) override;
	virtual void SetMesh(SkeletalMesh* skeletalMesh) override;

	void SetRenderOperations(RenderPassType renderPassType = RenderPassType::Main);

	void PlayAnimation(const std::string& animationName, const PlayLoopData& playLoopData = { false, {} }, const KeyframeData& keyframeData = {});

	void AttachBoneToMatrixPointer(const std::string& boneName, const Matrix* matrix);
	void RemoveBoneToMatrixPointer(const std::string& boneName);

	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;

	SocketComponent* AddSocketToBone(const std::string& boneName);
	SocketComponent* GetSocket(const std::string& boneName);
protected:

private:
	SkeletalMeshAnimation skeletalMeshAnimation_;

	std::vector<Matrix> boneTransformations_;
	std::unordered_map<std::string, SocketComponent*> sockets_;
	std::unordered_map<int, const Matrix*> boneIdToAttachedMatrixPointerMap_;
};

#endif