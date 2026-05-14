#include "pch.h"

#include "AnimationPose.h"

void AnimationPose::Resize(size_t boneCount)
{
	localTransforms.resize(boneCount);
}

void AnimationPose::SetToBindPose(const std::vector<AnimationTransform>& bindPose)
{
	localTransforms = bindPose;
}

const AnimationTrack* AnimationClip::GetTrack(int boneIndex) const
{
	const auto iterator = boneIndexToTrackIndex_.find(boneIndex);
	if (iterator == boneIndexToTrackIndex_.end())
	{
		return nullptr;
	}

	return &tracks[iterator->second];
}

AnimationTrack* AnimationClip::GetTrack(int boneIndex)
{
	const auto iterator = boneIndexToTrackIndex_.find(boneIndex);
	if (iterator == boneIndexToTrackIndex_.end())
	{
		return nullptr;
	}

	return &tracks[iterator->second];
}

void AnimationClip::RebuildTrackLookup()
{
	boneIndexToTrackIndex_.clear();
	for (size_t trackIndex = 0; trackIndex < tracks.size(); ++trackIndex)
	{
		if (0 <= tracks[trackIndex].boneIndex)
		{
			boneIndexToTrackIndex_[tracks[trackIndex].boneIndex] = trackIndex;
		}
	}
}

void AnimationSkeleton::Resize(size_t boneCount)
{
	boneNames.resize(boneCount);
	parentIndices.assign(boneCount, -1);
	inverseBindMatrices.assign(boneCount, Matrix::IdentityMatrix);
	bindLocalPose.resize(boneCount);
	evaluationOrder.clear();
	evaluationOrder.reserve(boneCount);
}

bool AnimationSkeleton::IsValid() const
{
	const size_t boneCount = boneNames.size();
	return boneCount > 0 &&
		parentIndices.size() == boneCount &&
		inverseBindMatrices.size() == boneCount &&
		bindLocalPose.size() == boneCount;
}
