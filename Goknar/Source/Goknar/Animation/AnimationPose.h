#ifndef __ANIMATIONPOSE_H__
#define __ANIMATIONPOSE_H__

#include "Goknar/Core.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Math/Matrix.h"
#include "Goknar/Math/Quaternion.h"

#include <string>
#include <unordered_map>
#include <vector>

struct GOKNAR_API AnimationTransform
{
	Vector3 translation{ Vector3::ZeroVector };
	Quaternion rotation{ Quaternion::Identity };
	Vector3 scale{ Vector3(1.f) };

	Matrix ToMatrix() const
	{
		return Matrix::GetTransformationMatrix(rotation, translation, scale);
	}
};

struct GOKNAR_API AnimationPose
{
	void Resize(size_t boneCount);
	void SetToBindPose(const std::vector<AnimationTransform>& bindPose);

	std::vector<AnimationTransform> localTransforms{};
};

struct GOKNAR_API AnimationVectorSample
{
	float time{ 0.f };
	Vector3 value{ Vector3::ZeroVector };
};

struct GOKNAR_API AnimationQuaternionSample
{
	float time{ 0.f };
	Quaternion value{ Quaternion::Identity };
};

struct GOKNAR_API AnimationTrack
{
	int boneIndex{ -1 };
	std::vector<AnimationVectorSample> translationKeys{};
	std::vector<AnimationQuaternionSample> rotationKeys{};
	std::vector<AnimationVectorSample> scaleKeys{};
};

struct GOKNAR_API AnimationEvent
{
	float time{ 0.f };
	std::string name{};
	std::string payload{};
};

struct GOKNAR_API AnimationClip
{
	const AnimationTrack* GetTrack(int boneIndex) const;
	AnimationTrack* GetTrack(int boneIndex);
	void RebuildTrackLookup();

	std::string name{};
	float duration{ 0.f };
	float ticksPerSecond{ 30.f };
	bool loop{ true };

	std::vector<AnimationTrack> tracks{};
	std::vector<AnimationEvent> events{};

private:
	std::unordered_map<int, size_t> boneIndexToTrackIndex_{};
};

struct GOKNAR_API AnimationSkeleton
{
	void Resize(size_t boneCount);
	bool IsValid() const;

	std::vector<std::string> boneNames{};
	std::vector<int> parentIndices{};
	std::vector<int> evaluationOrder{};
	std::vector<Matrix> inverseBindMatrices{};
	std::vector<AnimationTransform> bindLocalPose{};
	std::unordered_map<std::string, int> boneNameToIndex{};
};

#endif
