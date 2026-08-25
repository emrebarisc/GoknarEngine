#include "pch.h"

#include "SkeletalMeshInstance.h"

#include <algorithm>
#include <cmath>
#include <execution>

#include "Goknar/Engine.h"
#include "Goknar/Animation/AnimationGraph.h"
#include "Goknar/Animation/AnimationNode.h"
#include "Goknar/Animation/AnimationRuntime.h"
#include "Goknar/Components/SocketComponent.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/ShaderTypes.h"

namespace
{
	float SmoothBlendParameter(float current, float target, float smoothingSpeed, float deltaTime)
	{
		if (smoothingSpeed <= 0.f || deltaTime <= 0.f)
		{
			return target;
		}

		const float alpha = 1.f - std::exp(-smoothingSpeed * deltaTime);
		return GoknarMath::Lerp(current, target, GoknarMath::Clamp(alpha, 0.f, 1.f));
	}

	float SmoothBlendAlpha(float alpha)
	{
		alpha = GoknarMath::Clamp(alpha, 0.f, 1.f);
		return alpha * alpha * (3.f - 2.f * alpha);
	}

	Matrix ApplyModifierAroundBonePosition(const Matrix& modelSpaceMatrix, const Matrix& modifier)
	{
		const Vector3 bonePosition = modelSpaceMatrix.GetTranslation();
		return Matrix::GetPositionMatrix(bonePosition) * modifier * Matrix::GetPositionMatrix(-bonePosition) * modelSpaceMatrix;
	}
}

SkeletalMeshInstance::SkeletalMeshInstance(RenderComponent* parentComponent) :
	IMeshInstance(parentComponent)
{
}

SkeletalMeshInstance::~SkeletalMeshInstance()
{
}

void SkeletalMeshInstance::PrepareForTheCurrentFrame()
{
	if (!mesh_)
	{
		return;
	}

	SkeletalMesh* LOD0 = mesh_->GetLOD(0);

	if (!LOD0)
	{
		return;
	}

	LOD0->BuildRuntimeAnimationData();

	if (!hasGraphPose_)
	{
		SampleDirectAnimationToLocalPose();
	}

	BuildMatricesAndUpdateSockets();
	graphPoseWasUpdatedThisFrame_ = false;
}

void SkeletalMeshInstance::BuildMatricesAndUpdateSockets()
{
	if (!mesh_)
	{
		return;
	}

	SkeletalMesh* LOD0 = mesh_->GetLOD(0);

	if (!LOD0)
	{
		return;
	}

	const AnimationSkeleton& skeleton = LOD0->GetAnimationSkeleton();
	if (localPose_.localTransforms.size() != skeleton.bindLocalPose.size())
	{
		localPose_.SetToBindPose(skeleton.bindLocalPose);
	}

	const size_t boneCount = skeleton.boneNames.size();
	modelSpaceBoneTransformations_.assign(boneCount, Matrix::IdentityMatrix);
	boneTransformations_.assign(boneCount, Matrix::IdentityMatrix);

	if (!skeleton.IsValid() || localPose_.localTransforms.size() < boneCount)
	{
		return;
	}

	const Matrix componentToWorldInverse = parentComponent_->GetComponentToWorldTransformationMatrix().GetInverse();

	for (int boneIndex : skeleton.evaluationOrder)
	{
		if (boneIndex < 0 || boneCount <= (size_t)boneIndex)
		{
			continue;
		}

		Matrix localMatrix = localPose_.localTransforms[boneIndex].ToMatrix();
		const int parentIndex = skeleton.parentIndices[boneIndex];
		const Matrix parentMatrix = 0 <= parentIndex ? modelSpaceBoneTransformations_[parentIndex] : Matrix::IdentityMatrix;
		Matrix modelSpaceMatrix = parentMatrix * localMatrix;

		const auto binderIterator = boneIdToMatrixBinderMap_.find(boneIndex);
		if (binderIterator != boneIdToMatrixBinderMap_.end())
		{
			const BoneToMatrixBinder& binder = binderIterator->second;
			if (binder.matrix)
			{
				if (binder.type == BoneToMatrixBineType::Relative)
				{
					modelSpaceMatrix = ApplyModifierAroundBonePosition(modelSpaceMatrix, *binder.matrix);
				}
				else if (binder.type == BoneToMatrixBineType::World)
				{
					modelSpaceMatrix = ApplyModifierAroundBonePosition(
						modelSpaceMatrix,
						componentToWorldInverse * *binder.matrix * parentComponent_->GetComponentToWorldTransformationMatrix());
				}
			}
		}

		modelSpaceBoneTransformations_[boneIndex] = modelSpaceMatrix;
		boneTransformations_[boneIndex] = modelSpaceMatrix * skeleton.inverseBindMatrices[boneIndex];
	}

	UpdateSocketsFromModelSpacePose();
}

void SkeletalMeshInstance::UpdateSocketsFromModelSpacePose()
{
	if (!mesh_)
	{
		return;
	}

	SkeletalMesh* LOD0 = mesh_->GetLOD(0);

	if (!LOD0)
	{
		return;
	}

	for (const auto& socketPair : sockets_)
	{
		const int boneIndex = LOD0->FindBoneId(socketPair.first);
		if (boneIndex < 0 || (size_t)boneIndex >= modelSpaceBoneTransformations_.size())
		{
			continue;
		}

		socketPair.second->SetBoneTransformationMatrix(modelSpaceBoneTransformations_[boneIndex]);
	}
}

void SkeletalMeshInstance::SampleDirectAnimationToLocalPose()
{
	if (!mesh_)
	{
		return;
	}

	SkeletalMesh* LOD0 = mesh_->GetLOD(0);

	if (!LOD0)
	{
		return;
	}

	const AnimationSkeleton& skeleton = LOD0->GetAnimationSkeleton();
	if (!skeleton.IsValid())
	{
		return;
	}

	if (!skeletalMeshAnimation_.skeletalAnimation)
	{
		localPose_.SetToBindPose(skeleton.bindLocalPose);
		return;
	}

	const AnimationClip* clip = LOD0->GetAnimationClip(skeletalMeshAnimation_.skeletalAnimation->name);
	if (!clip)
	{
		localPose_.SetToBindPose(skeleton.bindLocalPose);
		return;
	}

	AnimationRuntime::SampleClip(*clip, skeleton, skeletalMeshAnimation_.animationTime, localPose_, !skeletalMeshAnimation_.playLoopData.playOnce);
}

void SkeletalMeshInstance::PrepareForTheNextFrame()
{
	auto& animation = skeletalMeshAnimation_;
	if (!animation.skeletalAnimation)
	{
		return;
	}

	const float durationInSeconds = animation.skeletalAnimation->duration;
	const float fps = animation.skeletalAnimation->ticksPerSecond;

	const float newElapsedTime = engine->GetElapsedTime() - animation.initialTimeInSeconds;

	float newAnimationTime = std::fmod(newElapsedTime, durationInSeconds);

	const bool loopedThisFrame = newAnimationTime < animation.animationTime;

	if (loopedThisFrame && animation.playLoopData.playOnce)
	{
		if (!animation.playLoopData.callback.isNull())
		{
			animation.playLoopData.callback();
		}
		return;
	}

	animation.elapsedTimeInSeconds = newElapsedTime;
	float oldAnimationTime = animation.animationTime;
	animation.animationTime = newAnimationTime;

	if (loopedThisFrame && !animation.playLoopData.callback.isNull())
	{
		animation.playLoopData.callback();
	}

	int startFrameIndex = (int)(oldAnimationTime * fps);
	int endFrameIndex = (int)(newAnimationTime * fps);

	if (startFrameIndex != endFrameIndex)
	{
		auto& callbackMap = animation.keyframeData.keyframeCallbackMap;

		std::function<void(int, int)> triggerRange =
			[&](int from, int to)
			{
				for (int i = from; i <= to; ++i)
				{
					auto it = callbackMap.find(i);
					if (it != callbackMap.end())
					{
						it->second();
					}
				}
			};

		if (loopedThisFrame)
		{
			int maxFrames = (int)(durationInSeconds * fps);
			triggerRange(startFrameIndex + 1, maxFrames);
			triggerRange(0, endFrameIndex);
		}
		else
		{
			triggerRange(startFrameIndex + 1, endFrameIndex);
		}

		animation.currentKeyframe = endFrameIndex;
	}
}

void SkeletalMeshInstance::Render(RenderPassType renderPassType, int subMeshIndex, int LODIndex/* = 0*/)
{
	SetRenderOperations(renderPassType, LODIndex, subMeshIndex);
}

void SkeletalMeshInstance::SetRenderOperations(RenderPassType renderPassType, int subMeshIndex, int LODIndex/* = 0*/)
{
	const std::vector<SkeletalMeshUnit*>& subMeshes = mesh_->GetLOD(LODIndex)->GetSubMeshes();
	if (0 <= subMeshIndex && subMeshIndex < (int)subMeshes.size())
	{
		IMaterialBase* material = GetMaterial(subMeshIndex);
		Shader* shader = material ? material->GetShader(renderPassType) : nullptr;
		if (shader)
		{
			shader->SetMatrixVector(SHADER_VARIABLE_NAMES::SKELETAL_MESH::BONES, boneTransformations_);
		}
	}
	IMeshInstance::Render(renderPassType, subMeshIndex, LODIndex);
}

void SkeletalMeshInstance::SetMesh(SkeletalMeshContainer* skeletalMesh)
{
	IMeshInstance::SetMesh(skeletalMesh);

	SkeletalMesh* LOD0 = skeletalMesh->GetLOD(0);

	LOD0->BuildRuntimeAnimationData();
	boneTransformations_.resize(LOD0->GetBoneSize(), Matrix::IdentityMatrix);
	modelSpaceBoneTransformations_.resize(LOD0->GetBoneSize(), Matrix::IdentityMatrix);
	localPose_.SetToBindPose(LOD0->GetAnimationSkeleton().bindLocalPose);
	graphPose_.SetToBindPose(LOD0->GetAnimationSkeleton().bindLocalPose);
	crossfadePose_.SetToBindPose(LOD0->GetAnimationSkeleton().bindLocalPose);
	blendPoseA_.SetToBindPose(LOD0->GetAnimationSkeleton().bindLocalPose);
	blendPoseB_.SetToBindPose(LOD0->GetAnimationSkeleton().bindLocalPose);
}

void SkeletalMeshInstance::PlayAnimation(const std::string& animationName, const PlayLoopData& playLoopData/* = { false, {} }*/, const KeyframeData& keyframeData/* = {}*/)
{
	if (animationName.empty() ||
		(skeletalMeshAnimation_.skeletalAnimation &&
		skeletalMeshAnimation_.skeletalAnimation->name == animationName))
	{
		return;
	}

	SkeletalMesh* skeletalMesh = mesh_->GetLOD(0);
	if (!skeletalMesh)
	{
		return;
	}

	skeletalMeshAnimation_.skeletalAnimation = skeletalMesh->GetSkeletalAnimation(animationName);
	if (!skeletalMeshAnimation_.skeletalAnimation)
	{
		return;
	}
	skeletalMeshAnimation_.name = animationName;
	skeletalMeshAnimation_.animationTime = 0.f;
	skeletalMeshAnimation_.elapsedTimeInSeconds = 0.f;
	skeletalMeshAnimation_.initialTimeInSeconds = engine->GetElapsedTime();

	skeletalMeshAnimation_.playLoopData = playLoopData;
	skeletalMeshAnimation_.keyframeData = keyframeData;
	hasGraphPose_ = false;
}

void SkeletalMeshInstance::EvaluateAnimationGraph(AnimationGraph& animationGraph, float deltaTime)
{
	if (!mesh_)
	{
		return;
	}

	SkeletalMesh* LOD0 = mesh_->GetLOD(0);

	if (!LOD0)
	{
		return;
	}

	LOD0->BuildRuntimeAnimationData();

	const std::shared_ptr<AnimationNode>& currentNode = animationGraph.GetCurrentNode();
	if (!currentNode)
	{
		localPose_.SetToBindPose(LOD0->GetAnimationSkeleton().bindLocalPose);
		hasGraphPose_ = true;
		graphPoseWasUpdatedThisFrame_ = true;
		return;
	}

	EvaluateAnimationNode(animationGraph, currentNode.get(), deltaTime, graphPose_);

	const std::shared_ptr<AnimationNode>& crossfadeSourceNode = animationGraph.GetCrossfadeSourceNode();
	if (animationGraph.IsCrossfading() && crossfadeSourceNode)
	{
		EvaluateAnimationNode(animationGraph, crossfadeSourceNode.get(), deltaTime, crossfadePose_);
		AnimationRuntime::BlendPoses(crossfadePose_, graphPose_, animationGraph.GetCrossfadeAlpha(), localPose_);
	}
	else
	{
		localPose_ = graphPose_;
	}

	hasGraphPose_ = true;
	graphPoseWasUpdatedThisFrame_ = true;
}

void SkeletalMeshInstance::EvaluateAnimationNode(AnimationGraph& animationGraph, AnimationNode* node, float deltaTime, AnimationPose& outPose)
{
	SkeletalMesh* LOD0 = mesh_->GetLOD(0);

	if (!LOD0)
	{
		return;
	}

	if (!node)
	{
		outPose.SetToBindPose(LOD0->GetAnimationSkeleton().bindLocalPose);
		return;
	}

	switch (node->type)
	{
	case AnimationNodeType::BlendSpace1D:
		EvaluateBlendSpace1DNode(animationGraph, node, deltaTime, outPose);
		break;
	case AnimationNodeType::BlendSpace2D:
		EvaluateBlendSpace2DNode(animationGraph, node, deltaTime, outPose);
		break;
	case AnimationNodeType::Clip:
	default:
		EvaluateClipNode(animationGraph, node, deltaTime, outPose);
		break;
	}
}

void SkeletalMeshInstance::EvaluateClipNode(AnimationGraph& animationGraph, AnimationNode* node, float deltaTime, AnimationPose& outPose)
{
	SkeletalMesh* LOD0 = mesh_->GetLOD(0);

	if (!LOD0)
	{
		return;
	}

	const AnimationSkeleton& skeleton = LOD0->GetAnimationSkeleton();
	AnimationNodeRuntimeData& runtimeData = animationGraph.GetRuntimeData(node);
	const AnimationClip* clip = LOD0->GetAnimationClip(node->animationName);
	if (!clip)
	{
		runtimeData.finished = true;
		outPose.SetToBindPose(skeleton.bindLocalPose);
		return;
	}

	runtimeData.time += deltaTime * node->playRate;
	if (node->loop)
	{
		runtimeData.time = AnimationRuntime::WrapClipTime(*clip, runtimeData.time, true);
		runtimeData.finished = false;
	}
	else
	{
		if (clip->duration <= runtimeData.time)
		{
			runtimeData.time = clip->duration;
			runtimeData.finished = true;
		}
	}

	AnimationRuntime::SampleClip(*clip, skeleton, runtimeData.time, outPose, node->loop);
}

void SkeletalMeshInstance::EvaluateBlendSpace1DNode(AnimationGraph& animationGraph, AnimationNode* node, float deltaTime, AnimationPose& outPose)
{
	SkeletalMesh* LOD0 = mesh_->GetLOD(0);

	if (!LOD0)
	{
		return;
	}

	const AnimationSkeleton& skeleton = LOD0->GetAnimationSkeleton();
	AnimationNodeRuntimeData& runtimeData = animationGraph.GetRuntimeData(node);
	if (node->blendSpace1DPoints.empty())
	{
		runtimeData.finished = true;
		outPose.SetToBindPose(skeleton.bindLocalPose);
		return;
	}

	std::vector<BlendSpace1DPoint> sortedPoints = node->blendSpace1DPoints;
	std::sort(
		sortedPoints.begin(),
		sortedPoints.end(),
		[](const BlendSpace1DPoint& first, const BlendSpace1DPoint& second)
		{
			return first.value < second.value;
		});

	const float rawParameter = animationGraph.GetFloatVariable(node->parameterName, 0.f);
	if (!runtimeData.hasSmoothedParameters)
	{
		runtimeData.smoothedParameter = rawParameter;
		runtimeData.hasSmoothedParameters = true;
	}
	else
	{
		runtimeData.smoothedParameter = SmoothBlendParameter(
			runtimeData.smoothedParameter,
			rawParameter,
			node->parameterSmoothingSpeed,
			deltaTime);
	}

	const float parameter = runtimeData.smoothedParameter;
	const BlendSpace1DPoint* firstPoint = &sortedPoints.front();
	const BlendSpace1DPoint* secondPoint = firstPoint;
	float blendAlpha = 0.f;

	if (parameter <= sortedPoints.front().value)
	{
		firstPoint = &sortedPoints.front();
		secondPoint = firstPoint;
	}
	else if (sortedPoints.back().value <= parameter)
	{
		firstPoint = &sortedPoints.back();
		secondPoint = firstPoint;
	}
	else
	{
		for (size_t pointIndex = 0; pointIndex + 1 < sortedPoints.size(); ++pointIndex)
		{
			if (sortedPoints[pointIndex].value <= parameter && parameter <= sortedPoints[pointIndex + 1].value)
			{
				firstPoint = &sortedPoints[pointIndex];
				secondPoint = &sortedPoints[pointIndex + 1];
				const float range = secondPoint->value - firstPoint->value;
				blendAlpha = range > 0.f ? (parameter - firstPoint->value) / range : 0.f;
				blendAlpha = SmoothBlendAlpha(blendAlpha);
				break;
			}
		}
	}

	const AnimationClip* firstClip = LOD0->GetAnimationClip(firstPoint->animationName);
	const AnimationClip* secondClip = LOD0->GetAnimationClip(secondPoint->animationName);
	if (!firstClip && !secondClip)
	{
		runtimeData.finished = true;
		outPose.SetToBindPose(skeleton.bindLocalPose);
		return;
	}

	if (!firstClip)
	{
		firstClip = secondClip;
		firstPoint = secondPoint;
		blendAlpha = 0.f;
	}
	if (!secondClip)
	{
		secondClip = firstClip;
		secondPoint = firstPoint;
		blendAlpha = 0.f;
	}

	const float blendedDuration = GoknarMath::Max(
		GoknarMath::Lerp(firstClip->duration, secondClip->duration, blendAlpha),
		0.0001f);

	runtimeData.normalizedTime += (deltaTime * node->playRate) / blendedDuration;
	if (node->loop)
	{
		runtimeData.normalizedTime = runtimeData.normalizedTime - std::floor(runtimeData.normalizedTime);
		runtimeData.finished = false;
	}
	else if (1.f <= runtimeData.normalizedTime)
	{
		runtimeData.normalizedTime = 1.f;
		runtimeData.finished = true;
	}

	const float firstTime = runtimeData.normalizedTime * firstClip->duration;
	const float secondTime = runtimeData.normalizedTime * secondClip->duration;

	if (firstClip == secondClip || blendAlpha <= 0.f)
	{
		AnimationRuntime::SampleClip(*firstClip, skeleton, firstTime, outPose, node->loop);
		return;
	}

	AnimationRuntime::SampleClip(*firstClip, skeleton, firstTime, blendPoseA_, node->loop);
	AnimationRuntime::SampleClip(*secondClip, skeleton, secondTime, blendPoseB_, node->loop);
	AnimationRuntime::BlendPoses(blendPoseA_, blendPoseB_, blendAlpha, outPose);
}

void SkeletalMeshInstance::EvaluateBlendSpace2DNode(AnimationGraph& animationGraph, AnimationNode* node, float deltaTime, AnimationPose& outPose)
{
	SkeletalMesh* LOD0 = mesh_->GetLOD(0);

	if (!LOD0)
	{
		return;
	}

	const AnimationSkeleton& skeleton = LOD0->GetAnimationSkeleton();
	AnimationNodeRuntimeData& runtimeData = animationGraph.GetRuntimeData(node);
	if (node->blendSpace2DPoints.empty())
	{
		runtimeData.finished = true;
		outPose.SetToBindPose(skeleton.bindLocalPose);
		return;
	}

	struct WeightedClip
	{
		const AnimationClip* clip{ nullptr };
		float weight{ 0.f };
		float distanceSquared{ 0.f };
	};

	const float rawParameterX = animationGraph.GetFloatVariable(node->parameterXName, 0.f);
	const float rawParameterY = animationGraph.GetFloatVariable(node->parameterYName, 0.f);
	if (!runtimeData.hasSmoothedParameters)
	{
		runtimeData.smoothedParameterX = rawParameterX;
		runtimeData.smoothedParameterY = rawParameterY;
		runtimeData.hasSmoothedParameters = true;
	}
	else
	{
		runtimeData.smoothedParameterX = SmoothBlendParameter(
			runtimeData.smoothedParameterX,
			rawParameterX,
			node->parameterSmoothingSpeed,
			deltaTime);
		runtimeData.smoothedParameterY = SmoothBlendParameter(
			runtimeData.smoothedParameterY,
			rawParameterY,
			node->parameterSmoothingSpeed,
			deltaTime);
	}

	const float parameterX = runtimeData.smoothedParameterX;
	const float parameterY = runtimeData.smoothedParameterY;

	std::vector<WeightedClip> weightedClips;
	weightedClips.reserve(node->blendSpace2DPoints.size());

	for (const BlendSpace2DPoint& point : node->blendSpace2DPoints)
	{
		const AnimationClip* clip = LOD0->GetAnimationClip(point.animationName);
		if (!clip)
		{
			continue;
		}

		const float dx = parameterX - point.x;
		const float dy = parameterY - point.y;
		const float distanceSquared = dx * dx + dy * dy;
		if (distanceSquared <= 0.000001f)
		{
			runtimeData.normalizedTime += clip->duration > 0.f ? (deltaTime * node->playRate) / clip->duration : 0.f;
			if (node->loop)
			{
				runtimeData.normalizedTime = runtimeData.normalizedTime - std::floor(runtimeData.normalizedTime);
				runtimeData.finished = false;
			}
			else if (1.f <= runtimeData.normalizedTime)
			{
				runtimeData.normalizedTime = 1.f;
				runtimeData.finished = true;
			}

			AnimationRuntime::SampleClip(*clip, skeleton, runtimeData.normalizedTime * clip->duration, outPose, node->loop);
			return;
		}

		weightedClips.push_back({ clip, 1.f / distanceSquared, distanceSquared });
	}

	if (weightedClips.empty())
	{
		runtimeData.finished = true;
		outPose.SetToBindPose(skeleton.bindLocalPose);
		return;
	}

	std::sort(
		weightedClips.begin(),
		weightedClips.end(),
		[](const WeightedClip& first, const WeightedClip& second)
		{
			return first.distanceSquared < second.distanceSquared;
		});

	if (weightedClips.size() > 4)
	{
		weightedClips.resize(4);
	}

	float totalWeight = 0.f;
	float weightedDuration = 0.f;
	for (const WeightedClip& weightedClip : weightedClips)
	{
		totalWeight += weightedClip.weight;
		weightedDuration += weightedClip.weight * weightedClip.clip->duration;
	}

	if (totalWeight <= 0.f)
	{
		runtimeData.finished = true;
		outPose.SetToBindPose(skeleton.bindLocalPose);
		return;
	}

	const float blendedDuration = GoknarMath::Max(weightedDuration / totalWeight, 0.0001f);
	runtimeData.normalizedTime += (deltaTime * node->playRate) / blendedDuration;
	if (node->loop)
	{
		runtimeData.normalizedTime = runtimeData.normalizedTime - std::floor(runtimeData.normalizedTime);
		runtimeData.finished = false;
	}
	else if (1.f <= runtimeData.normalizedTime)
	{
		runtimeData.normalizedTime = 1.f;
		runtimeData.finished = true;
	}

	const AnimationClip* firstClip = weightedClips.front().clip;
	AnimationRuntime::SampleClip(*firstClip, skeleton, runtimeData.normalizedTime * firstClip->duration, outPose, node->loop);

	float accumulatedWeight = weightedClips.front().weight;
	for (size_t clipIndex = 1; clipIndex < weightedClips.size(); ++clipIndex)
	{
		const WeightedClip& weightedClip = weightedClips[clipIndex];
		AnimationRuntime::SampleClip(*weightedClip.clip, skeleton, runtimeData.normalizedTime * weightedClip.clip->duration, blendPoseB_, node->loop);

		const float nextAccumulatedWeight = accumulatedWeight + weightedClip.weight;
		const float alpha = nextAccumulatedWeight > 0.f ? weightedClip.weight / nextAccumulatedWeight : 0.f;
		AnimationRuntime::BlendPoses(outPose, blendPoseB_, alpha, blendPoseA_);
		outPose = blendPoseA_;
		accumulatedWeight = nextAccumulatedWeight;
	}
}

void SkeletalMeshInstance::AttachBoneToMatrixPointer(const BoneToMatrixBinder& binder)
{
	if (!GetMesh())
	{
		return;
	}

	SkeletalMesh* LOD0 = mesh_->GetLOD(0);

	if (!LOD0)
	{
		return;
	}

	const int boneId = LOD0->FindBoneId(binder.boneName);
	if (boneId < 0)
	{
		return;
	}

	if (binder.matrix && binder.type != BoneToMatrixBineType::None)
	{
		boneIdToMatrixBinderMap_[boneId] = binder;
	}
	else
	{
		RemoveBoneToMatrixPointer(binder.boneName);
	}
}

void SkeletalMeshInstance::AttachBoneToMatrixPointer(const std::string& boneName, Matrix* matrix, BoneToMatrixBineType type)
{
	AttachBoneToMatrixPointer({ boneName, type, matrix });
}

void SkeletalMeshInstance::RemoveBoneToMatrixPointer(const std::string& boneName)
{
	if (!GetMesh())
	{
		return;
	}

	SkeletalMesh* LOD0 = mesh_->GetLOD(0);

	if (!LOD0)
	{
		return;
	}

	const int boneId = LOD0->FindBoneId(boneName);
	if (boneId < 0)
	{
		return;
	}

	decltype(boneIdToMatrixBinderMap_)::iterator iterator = boneIdToMatrixBinderMap_.find(boneId);

	if (iterator == boneIdToMatrixBinderMap_.end())
	{
		return;
	}

	boneIdToMatrixBinderMap_.erase(iterator);
}

void SkeletalMeshInstance::AddMeshInstanceToRenderer()
{
	engine->GetRenderer()->AddSkeletalMeshInstance(this);
}

void SkeletalMeshInstance::RemoveMeshInstanceFromRenderer()
{
	engine->GetRenderer()->RemoveSkeletalMeshInstance(this);
}

SocketComponent* SkeletalMeshInstance::AddSocketToBone(const std::string& boneName)
{
	if (sockets_.find(boneName) == sockets_.end())
	{
		SocketComponent* socketComponent = new SocketComponent(parentComponent_);
		socketComponent->SetOwner(parentComponent_->GetOwner());
		sockets_[boneName] = socketComponent;
	}

	return sockets_[boneName];
}

SocketComponent* SkeletalMeshInstance::GetSocket(const std::string& boneName)
{
	if (sockets_.find(boneName) != sockets_.end())
	{
		return sockets_[boneName];
	}

	return nullptr;
}
