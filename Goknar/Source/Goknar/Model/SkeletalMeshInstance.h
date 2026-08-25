#ifndef __SKELETALMESHINSTANCE_H__
#define __SKELETALMESHINSTANCE_H__

#include "IMeshInstance.h"

#include <unordered_map>

#include "Delegates/Delegate.h"
#include "Goknar/Animation/AnimationPose.h"
#include "Model/SkeletalMesh.h"

struct AnimationGraph;
struct AnimationNode;
struct SkeletalAnimation;
class SocketComponent;

struct GOKNAR_API PlayLoopData
{
	bool playOnce{ false };
	Delegate<void()> callback{};
};

struct GOKNAR_API KeyframeData
{
	void AddCallbackToKeyframe(int keyframe, const Delegate<void()>& callback)
	{
		keyframeCallbackMap.insert({ keyframe, callback });
	}

	std::unordered_map<int, Delegate<void()>> keyframeCallbackMap{};
};

struct GOKNAR_API SkeletalMeshAnimation
{
	std::string name{ "" };
	PlayLoopData playLoopData{};
	KeyframeData keyframeData{};
	const SkeletalAnimation* skeletalAnimation{ nullptr };
	float animationTime{ 0.f };
	float elapsedTimeInSeconds{ 0.f };
	float initialTimeInSeconds{ 0.f };
	int currentKeyframe{ 0 };
};

enum class BoneToMatrixBineType : unsigned char
{
	None = 0,
	Relative,
	World
};

struct GOKNAR_API BoneToMatrixBinder
{
	std::string boneName;
	BoneToMatrixBineType type{ BoneToMatrixBineType::None };
	Matrix* matrix{ nullptr };
};

class GOKNAR_API SkeletalMeshInstance : public IMeshInstance<SkeletalMeshContainer>
{
public:
	SkeletalMeshInstance() = delete;
	SkeletalMeshInstance(RenderComponent* parentComponent);
	virtual ~SkeletalMeshInstance();
	virtual void Render(RenderPassType renderPassType, int subMeshIndex = 0, int LODIndex = 0) override;
	virtual void SetMesh(SkeletalMeshContainer* skeletalMesh) override;

	void PlayAnimation(const std::string& animationName, const PlayLoopData& playLoopData = { false, {} }, const KeyframeData& keyframeData = {});

	void AttachBoneToMatrixPointer(const BoneToMatrixBinder& binder);
	void AttachBoneToMatrixPointer(const std::string& boneName, Matrix* matrix, BoneToMatrixBineType type = BoneToMatrixBineType::Relative);
	void RemoveBoneToMatrixPointer(const std::string& boneName);

	void PrepareForTheCurrentFrame();
	void PrepareForTheNextFrame();
	void EvaluateAnimationGraph(AnimationGraph& animationGraph, float deltaTime);

	void AddMeshInstanceToRenderer() override;
	void RemoveMeshInstanceFromRenderer() override;

	SocketComponent* AddSocketToBone(const std::string& boneName);
	SocketComponent* GetSocket(const std::string& boneName);

	const SkeletalMeshAnimation& GetSkeletalMeshAnimation() const
	{
		return skeletalMeshAnimation_;
	}

protected:

private:
	void SetRenderOperations(RenderPassType renderPassType, int subMeshIndex = 0, int LODIndex = 0);
	void SampleDirectAnimationToLocalPose();
	void BuildMatricesAndUpdateSockets();
	void UpdateSocketsFromModelSpacePose();
	void EvaluateAnimationNode(AnimationGraph& animationGraph, AnimationNode* node, float deltaTime, AnimationPose& outPose);
	void EvaluateClipNode(AnimationGraph& animationGraph, AnimationNode* node, float deltaTime, AnimationPose& outPose);
	void EvaluateBlendSpace1DNode(AnimationGraph& animationGraph, AnimationNode* node, float deltaTime, AnimationPose& outPose);
	void EvaluateBlendSpace2DNode(AnimationGraph& animationGraph, AnimationNode* node, float deltaTime, AnimationPose& outPose);

	SkeletalMeshAnimation skeletalMeshAnimation_{};

	std::vector<Matrix> boneTransformations_{};
	std::vector<Matrix> modelSpaceBoneTransformations_{};
	AnimationPose localPose_{};
	AnimationPose graphPose_{};
	AnimationPose crossfadePose_{};
	AnimationPose blendPoseA_{};
	AnimationPose blendPoseB_{};
	bool hasGraphPose_{ false };
	bool graphPoseWasUpdatedThisFrame_{ false };
	std::unordered_map<std::string, SocketComponent*> sockets_{};
	std::unordered_map<int, BoneToMatrixBinder> boneIdToMatrixBinderMap_{};
};

#endif
