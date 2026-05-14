#ifndef __ANIMATIONRUNTIME_H__
#define __ANIMATIONRUNTIME_H__

#include "Goknar/Core.h"
#include "Goknar/Animation/AnimationPose.h"

namespace AnimationRuntime
{
	GOKNAR_API float WrapClipTime(const AnimationClip& clip, float time, bool loop);
	GOKNAR_API void SampleClip(const AnimationClip& clip, const AnimationSkeleton& skeleton, float time, AnimationPose& outPose, bool loop = true);
	GOKNAR_API void BlendPoses(const AnimationPose& first, const AnimationPose& second, float alpha, AnimationPose& outPose);
	GOKNAR_API void BuildFinalBoneMatrices(
		const AnimationSkeleton& skeleton,
		const AnimationPose& localPose,
		std::vector<Matrix>& outModelSpaceMatrices,
		std::vector<Matrix>& outSkinMatrices);
}

#endif
