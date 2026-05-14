#include "pch.h"

#include "SkeletalMesh.h"

#include "Goknar/Engine.h"
#include "Goknar/Components/SocketComponent.h"

SkeletalMesh::SkeletalMesh() :
	Mesh<SkeletalMeshUnit>()
{
}

SkeletalMesh::~SkeletalMesh()
{
	int skeletalAnimationSize = skeletalAnimations_.size();
	for (unsigned int skeletalAnimationIndex = 0; skeletalAnimationIndex < skeletalAnimationSize; ++skeletalAnimationIndex)
	{
		delete skeletalAnimations_[skeletalAnimationIndex];
	}

	int bonesSize = bones_.size();
	for (unsigned int boneIndex = 0; boneIndex < bonesSize; ++boneIndex)
	{
		delete bones_[boneIndex];
	}

	delete boneNameToIdMap_;
	delete armature_;
}

void SkeletalMesh::PreInit()
{
	Mesh::PreInit();

	engine->AddSkeletalMeshToRenderer(this);
}

void SkeletalMesh::Init()
{
	Mesh::Init();
}

void SkeletalMesh::PostInit()
{
	Mesh::PostInit();
	BuildRuntimeAnimationData();
}

void SkeletalMesh::AddMesh(SkeletalMeshUnit* meshUnit)
{
	meshUnit->SetOwner(this);

	Mesh::AddMesh(meshUnit);
}

void SkeletalMesh::GetBoneTransforms(std::vector<Matrix>& transforms, const SkeletalAnimation* skeletalAnimation, float time, std::unordered_map<std::string, SocketComponent*>& socketMap)
{
	SetupTransforms(armature_->root, Matrix::IdentityMatrix, transforms, skeletalAnimation, time, socketMap);
}

void SkeletalMesh::SetupTransforms(Bone* bone, const Matrix& parentTransform, std::vector<Matrix>& transforms, const SkeletalAnimation* skeletalAnimation, float time, std::unordered_map<std::string, SocketComponent*>& socketMap)
{
	if (!bone)
	{
		return;
	}

	Vector3 interpolatedPosition = Vector3::ZeroVector;
	Vector3 interpolatedScaling = Vector3(1.f);
	Quaternion interpolatedRotation = Quaternion::Identity;

	Matrix boneTransformation = bone->transformation;
	if (skeletalAnimation)
	{
		SkeletalAnimationKeyframe* skeletalAnimationNode = skeletalAnimation->affectedBoneNameToSkeletalAnimationKeyframeMap.at(bone->name);

		if (skeletalAnimationNode)
		{
			skeletalAnimationNode->GetInterpolatedPosition(interpolatedPosition, time);
			skeletalAnimationNode->GetInterpolatedRotation(interpolatedRotation, time);
			skeletalAnimationNode->GetInterpolatedScaling(interpolatedScaling, time);

			boneTransformation = Matrix::GetTransformationMatrix(interpolatedRotation, interpolatedPosition, interpolatedScaling);
		}
	}

	Matrix globalTransformation = parentTransform * boneTransformation;

	Matrix transform = /*armature_->globalInverseTransform * */globalTransformation * bone->offset;

	if (socketMap.find(bone->name) != socketMap.end())
	{
		socketMap[bone->name]->SetBoneTransformationMatrix(globalTransformation);
	}

	transforms[(*boneNameToIdMap_)[bone->name]] = transform;
	unsigned int childrenSize = bone->children.size();
	for (unsigned int childIndex = 0; childIndex < childrenSize; ++childIndex)
	{
		SetupTransforms(bone->children[childIndex], globalTransformation, transforms, skeletalAnimation, time, socketMap);
	}
}

void SkeletalMesh::BuildRuntimeAnimationData()
{
	if (runtimeAnimationDataBuilt_)
	{
		return;
	}

	runtimeSkeleton_ = AnimationSkeleton();
	runtimeAnimationClips_.clear();
	nameToRuntimeAnimationClipIndexMap_.clear();

	if (!armature_ || !armature_->root || boneSize_ == 0)
	{
		return;
	}

	runtimeSkeleton_.Resize(boneSize_);
	AddBoneToRuntimeSkeleton(armature_->root, -1);
	BuildRuntimeAnimationClips();

	runtimeAnimationDataBuilt_ = true;
}

void SkeletalMesh::AddBoneToRuntimeSkeleton(Bone* bone, int parentIndex)
{
	if (!bone)
	{
		return;
	}

	const int boneIndex = FindBoneId(bone->name);
	if (boneIndex < 0 || (size_t)boneIndex >= runtimeSkeleton_.boneNames.size())
	{
		return;
	}

	Vector3 translation = Vector3::ZeroVector;
	Vector3 scale = Vector3(1.f);
	Quaternion rotation = Quaternion::Identity;
	bone->transformation.Decompose(translation, scale, rotation);

	runtimeSkeleton_.boneNames[boneIndex] = bone->name;
	runtimeSkeleton_.boneNameToIndex[bone->name] = boneIndex;
	runtimeSkeleton_.parentIndices[boneIndex] = parentIndex;
	runtimeSkeleton_.inverseBindMatrices[boneIndex] = bone->offset;
	runtimeSkeleton_.bindLocalPose[boneIndex] = AnimationTransform{ translation, rotation, scale };
	runtimeSkeleton_.evaluationOrder.push_back(boneIndex);

	for (Bone* child : bone->children)
	{
		AddBoneToRuntimeSkeleton(child, boneIndex);
	}
}

void SkeletalMesh::BuildRuntimeAnimationClips()
{
	runtimeAnimationClips_.reserve(skeletalAnimations_.size());

	for (const SkeletalAnimation* skeletalAnimation : skeletalAnimations_)
	{
		if (!skeletalAnimation)
		{
			continue;
		}

		AnimationClip clip;
		clip.name = skeletalAnimation->name;
		clip.duration = skeletalAnimation->duration;
		clip.ticksPerSecond = skeletalAnimation->ticksPerSecond;
		clip.loop = true;
		clip.tracks.reserve(skeletalAnimation->animationKeyframeCount);

		for (unsigned int keyframeIndex = 0; keyframeIndex < skeletalAnimation->animationKeyframeCount; ++keyframeIndex)
		{
			const SkeletalAnimationKeyframe* keyframe = skeletalAnimation->animationKeyframes[keyframeIndex];
			if (!keyframe)
			{
				continue;
			}

			const int boneIndex = FindBoneId(keyframe->affectedBoneName);
			if (boneIndex < 0)
			{
				continue;
			}

			AnimationTrack track;
			track.boneIndex = boneIndex;
			track.translationKeys.reserve(keyframe->positionKeySize);
			track.rotationKeys.reserve(keyframe->rotationKeySize);
			track.scaleKeys.reserve(keyframe->scalingKeySize);

			for (int positionIndex = 0; positionIndex < keyframe->positionKeySize; ++positionIndex)
			{
				const AnimationVectorKey& sourceKey = keyframe->positionKeys[positionIndex];
				track.translationKeys.push_back({ sourceKey.time, sourceKey.value });
			}

			for (int rotationIndex = 0; rotationIndex < keyframe->rotationKeySize; ++rotationIndex)
			{
				const AnimationQuaternionKey& sourceKey = keyframe->rotationKeys[rotationIndex];
				track.rotationKeys.push_back({ sourceKey.time, sourceKey.value });
			}

			for (int scaleIndex = 0; scaleIndex < keyframe->scalingKeySize; ++scaleIndex)
			{
				const AnimationVectorKey& sourceKey = keyframe->scalingKeys[scaleIndex];
				track.scaleKeys.push_back({ sourceKey.time, sourceKey.value });
			}

			clip.tracks.push_back(std::move(track));
		}

		clip.RebuildTrackLookup();
		nameToRuntimeAnimationClipIndexMap_[clip.name] = runtimeAnimationClips_.size();
		runtimeAnimationClips_.push_back(std::move(clip));
	}
}
