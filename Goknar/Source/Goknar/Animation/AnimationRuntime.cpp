#include "pch.h"

#include "AnimationRuntime.h"

#include <algorithm>
#include <cmath>

namespace
{
	template<typename KeyType>
	int FindPreviousKeyIndex(const std::vector<KeyType>& keys, float time)
	{
		if (keys.size() <= 1 || time <= keys.front().time)
		{
			return 0;
		}

		if (keys.back().time <= time)
		{
			return (int)keys.size() - 1;
		}

		const auto upper = std::upper_bound(
			keys.begin(),
			keys.end(),
			time,
			[](float value, const KeyType& key)
			{
				return value < key.time;
			});

		return (int)std::distance(keys.begin(), upper) - 1;
	}

	Vector3 SampleVector(const std::vector<AnimationVectorSample>& keys, float time, const Vector3& fallback)
	{
		if (keys.empty())
		{
			return fallback;
		}

		const int previousIndex = FindPreviousKeyIndex(keys, time);
		if (previousIndex >= (int)keys.size() - 1)
		{
			return keys[previousIndex].value;
		}

		const AnimationVectorSample& previous = keys[previousIndex];
		const AnimationVectorSample& next = keys[previousIndex + 1];
		const float duration = next.time - previous.time;
		const float alpha = duration > 0.f ? (time - previous.time) / duration : 0.f;
		return GoknarMath::Lerp(previous.value, next.value, GoknarMath::Clamp(alpha, 0.f, 1.f));
	}

	Quaternion SampleQuaternion(const std::vector<AnimationQuaternionSample>& keys, float time, const Quaternion& fallback)
	{
		if (keys.empty())
		{
			return fallback;
		}

		const int previousIndex = FindPreviousKeyIndex(keys, time);
		if (previousIndex >= (int)keys.size() - 1)
		{
			return keys[previousIndex].value;
		}

		const AnimationQuaternionSample& previous = keys[previousIndex];
		const AnimationQuaternionSample& next = keys[previousIndex + 1];
		const float duration = next.time - previous.time;
		const float alpha = duration > 0.f ? (time - previous.time) / duration : 0.f;
		return Quaternion::Slerp(previous.value, next.value, GoknarMath::Clamp(alpha, 0.f, 1.f)).GetNormalized();
	}
}

float AnimationRuntime::WrapClipTime(const AnimationClip& clip, float time, bool loop)
{
	if (clip.duration <= 0.f)
	{
		return 0.f;
	}

	if (!loop)
	{
		return GoknarMath::Clamp(time, 0.f, clip.duration);
	}

	float wrappedTime = std::fmod(time, clip.duration);
	if (wrappedTime < 0.f)
	{
		wrappedTime += clip.duration;
	}

	return wrappedTime;
}

void AnimationRuntime::SampleClip(const AnimationClip& clip, const AnimationSkeleton& skeleton, float time, AnimationPose& outPose, bool loop)
{
	outPose.SetToBindPose(skeleton.bindLocalPose);
	if (!skeleton.IsValid())
	{
		return;
	}

	const float clipTime = WrapClipTime(clip, time, loop);
	for (const AnimationTrack& track : clip.tracks)
	{
		if (track.boneIndex < 0 || (size_t)track.boneIndex >= outPose.localTransforms.size())
		{
			continue;
		}

		AnimationTransform& transform = outPose.localTransforms[track.boneIndex];
		transform.translation = SampleVector(track.translationKeys, clipTime, transform.translation);
		transform.rotation = SampleQuaternion(track.rotationKeys, clipTime, transform.rotation);
		transform.scale = SampleVector(track.scaleKeys, clipTime, transform.scale);
	}
}

void AnimationRuntime::BlendPoses(const AnimationPose& first, const AnimationPose& second, float alpha, AnimationPose& outPose)
{
	const size_t firstBoneCount = first.localTransforms.size();
	const size_t secondBoneCount = second.localTransforms.size();
	const size_t boneCount = firstBoneCount < secondBoneCount ? firstBoneCount : secondBoneCount;
	outPose.Resize(boneCount);

	const float clampedAlpha = GoknarMath::Clamp(alpha, 0.f, 1.f);
	for (size_t boneIndex = 0; boneIndex < boneCount; ++boneIndex)
	{
		const AnimationTransform& firstTransform = first.localTransforms[boneIndex];
		const AnimationTransform& secondTransform = second.localTransforms[boneIndex];
		AnimationTransform& outTransform = outPose.localTransforms[boneIndex];

		outTransform.translation = GoknarMath::Lerp(firstTransform.translation, secondTransform.translation, clampedAlpha);
		outTransform.rotation = Quaternion::Slerp(firstTransform.rotation, secondTransform.rotation, clampedAlpha).GetNormalized();
		outTransform.scale = GoknarMath::Lerp(firstTransform.scale, secondTransform.scale, clampedAlpha);
	}
}

void AnimationRuntime::BuildFinalBoneMatrices(
	const AnimationSkeleton& skeleton,
	const AnimationPose& localPose,
	std::vector<Matrix>& outModelSpaceMatrices,
	std::vector<Matrix>& outSkinMatrices)
{
	const size_t boneCount = skeleton.boneNames.size();
	outModelSpaceMatrices.assign(boneCount, Matrix::IdentityMatrix);
	outSkinMatrices.assign(boneCount, Matrix::IdentityMatrix);

	if (!skeleton.IsValid() || localPose.localTransforms.size() < boneCount)
	{
		return;
	}

	for (int boneIndex : skeleton.evaluationOrder)
	{
		if (boneIndex < 0 || (size_t)boneIndex >= boneCount)
		{
			continue;
		}

		const Matrix localMatrix = localPose.localTransforms[boneIndex].ToMatrix();
		const int parentIndex = skeleton.parentIndices[boneIndex];
		const Matrix parentMatrix = 0 <= parentIndex ? outModelSpaceMatrices[parentIndex] : Matrix::IdentityMatrix;

		outModelSpaceMatrices[boneIndex] = parentMatrix * localMatrix;
		outSkinMatrices[boneIndex] = outModelSpaceMatrices[boneIndex] * skeleton.inverseBindMatrices[boneIndex];
	}
}
