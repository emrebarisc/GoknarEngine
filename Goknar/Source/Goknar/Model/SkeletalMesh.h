#ifndef __SKELETALMESH_H__
#define __SKELETALMESH_H__

#include <string>
#include <unordered_map>
#include <vector>

#include "Mesh.h"
#include "SkeletalMeshUnit.h"

#include "Goknar/Core.h"
#include "Goknar/GoknarAssert.h"
#include "Goknar/Math/GoknarMath.h"
#include "Goknar/Math/Quaternion.h"

class SocketComponent;

struct GOKNAR_API Bone
{
    Bone(const std::string& n) : name(n) {}
    Bone(const std::string& n, const Matrix& o) : name(n), offset(o) {}
    Bone(const std::string& n, const Matrix& o, const Matrix& t) : name(n), offset(o), transformation(t) {}
    Bone(const Bone& rhs) : offset(rhs.offset), transformation(rhs.transformation) {}

    std::string name;

    Matrix offset{ Matrix::IdentityMatrix };
    Matrix transformation{ Matrix::IdentityMatrix };;

    std::vector<Bone*> children;
};

struct GOKNAR_API Armature
{
    Bone* root{ nullptr };
    Matrix globalInverseTransform{ Matrix::IdentityMatrix };
};

struct GOKNAR_API AnimationVectorKey
{
    AnimationVectorKey() {}

    AnimationVectorKey(float time, const Vector3& value) :
        time(time), 
        value(value)
    {}

    bool operator==(const AnimationVectorKey& rhs) const
    {
        return rhs.value == this->value;
    }
    bool operator!=(const AnimationVectorKey& rhs) const
    {
        return rhs.value != this->value;
    }

    bool operator<(const AnimationVectorKey& rhs) const
    {
        return time < rhs.time;
    }

    bool operator>(const AnimationVectorKey& rhs) const
    {
        return time > rhs.time;
    }

    float time{ 0.f };
    Vector3 value{ Vector3::ZeroVector };
};

struct GOKNAR_API AnimationQuaternionKey
{
    AnimationQuaternionKey() {}

    AnimationQuaternionKey(float t, const Quaternion& v) :
        time(t), 
        value(v) 
    {}

    bool operator==(const AnimationQuaternionKey& rhs) const
    {
        return rhs.value == this->value;
    }

    bool operator!=(const AnimationQuaternionKey& rhs) const
    {
        return rhs.value != this->value;
    }

    bool operator<(const AnimationQuaternionKey& rhs) const
    {
        return time < rhs.time;
    }

    bool operator>(const AnimationQuaternionKey& rhs) const
    {
        return time > rhs.time;
    }

    float time{ 0.0 };
    Quaternion value{ Quaternion::Identity };
};

struct GOKNAR_API SkeletalAnimationKeyframe
{
    ~SkeletalAnimationKeyframe()
    {
        delete[] rotationKeys;
        delete[] positionKeys;
        delete[] scalingKeys;
    }

    void GetInterpolatedScaling(Vector3& out, float time)
    {
        int previousIndex = 0;
        for (int scalingIndex = 0; scalingIndex < scalingKeySize; ++scalingIndex)
        {
            if (scalingKeys[scalingIndex].time < time)
            {
                previousIndex = scalingIndex;
            }
        }

        int nextIndex = previousIndex + 1;

        float alpha = (time - scalingKeys[previousIndex].time) / (scalingKeys[nextIndex].time - scalingKeys[previousIndex].time);

        out = GoknarMath::Lerp(scalingKeys[previousIndex].value, scalingKeys[nextIndex].value, alpha);
    }

    void GetInterpolatedScalingMatrix(Matrix& out, float time)
    {
        Vector3 scaling;
        GetInterpolatedScaling(scaling, time);
        out = Matrix::GetScalingMatrix(scaling);
    }

    void GetInterpolatedPosition(Vector3& out, float time)
    {
        int previousIndex = 0;
        for (int positionIndex = 0; positionIndex < positionKeySize; ++positionIndex)
        {
            if (positionKeys[positionIndex].time < time)
            {
                previousIndex = positionIndex;
            }
        }

        int nextIndex = previousIndex + 1;

        float alpha = (time - positionKeys[previousIndex].time) / (positionKeys[nextIndex].time - positionKeys[previousIndex].time);

        out = GoknarMath::Lerp(positionKeys[previousIndex].value, positionKeys[nextIndex].value, alpha);
    }

    void GetInterpolatedPositionMatrix(Matrix& out, float time)
    {
        Vector3 position;
        GetInterpolatedPosition(position, time);
        out = Matrix::GetPositionMatrix(position);
    }

    void GetInterpolatedRotation(Quaternion& out, float time)
    {
        int previousIndex = 0;
        for (int rotationIndex = 0; rotationIndex < (rotationKeySize - 1); ++rotationIndex)
        {
            if (rotationKeys[rotationIndex].time < time)
            {
                previousIndex = rotationIndex;
            }
        }

        int nextIndex = previousIndex + 1;

        float alpha = (time - rotationKeys[previousIndex].time) / (rotationKeys[nextIndex].time - rotationKeys[previousIndex].time);
        out = Quaternion::Slerp(rotationKeys[previousIndex].value, rotationKeys[nextIndex].value, alpha);
    }

    void GetInterpolatedRotationMatrix(Matrix& out, float time)
    {
        Quaternion rotation;
        GetInterpolatedRotation(rotation, time);
        out = rotation.GetMatrix();
    }

    std::string affectedBoneName;

    AnimationQuaternionKey* rotationKeys{ nullptr };
    AnimationVectorKey* positionKeys{ nullptr };
    AnimationVectorKey* scalingKeys{ nullptr };

    int rotationKeySize{ 0 };
    int positionKeySize{ 0 };
    int scalingKeySize{ 0 };
};

struct GOKNAR_API SkeletalAnimation
{
    ~SkeletalAnimation()
    {
        for (unsigned int animationKeyframeIndex = 0; animationKeyframeIndex < animationKeyframeCount; ++animationKeyframeIndex)
        {
            delete animationKeyframes[animationKeyframeIndex];
        }
        delete[] animationKeyframes;
    }

    void AddSkeletalAnimationKeyframe(int index, SkeletalAnimationKeyframe* skeletalAnimationKeyframe)
    {
        animationKeyframes[index] = skeletalAnimationKeyframe;
        affectedBoneNameToSkeletalAnimationKeyframeMap[skeletalAnimationKeyframe->affectedBoneName] = skeletalAnimationKeyframe;
    }

    std::unordered_map<std::string, SkeletalAnimationKeyframe*> affectedBoneNameToSkeletalAnimationKeyframeMap;
    SkeletalAnimationKeyframe** animationKeyframes{ nullptr };
    std::string name{ "" };
    float duration{ 0.f };
    float ticksPerSecond{ 0.f };
    unsigned int animationKeyframeCount{ 0 };
	unsigned int maxKeyframeCount{ 0 };
};

typedef std::unordered_map<std::string, unsigned int> BoneNameToIdMap;

typedef std::vector<SkeletalAnimation*> SkeletalAnimationVector;
typedef std::unordered_map<std::string, SkeletalAnimation*> NameToSkeletalAnimationMap;

class GOKNAR_API SkeletalMesh : public Mesh<SkeletalMeshUnit>
{
public:
	SkeletalMesh();

	virtual ~SkeletalMesh();

    virtual void PreInit() override;
    virtual void Init() override;
    virtual void PostInit() override;

    void AddBone(Bone* bone)
    {
        bones_.push_back(bone);
        ++boneSize_;
    }

    unsigned int GetBoneSize() const
    {
        return boneSize_;
    }
    
    int GetBoneId(const std::string& boneName)
    {
        int boneId = 0;

        if (boneNameToIdMap_->find(boneName) == boneNameToIdMap_->end())
        {
            boneId = boneNameToIdMapSize_;
            (*boneNameToIdMap_)[boneName] = boneId;
            ++boneNameToIdMapSize_;
        }
        else
        {
            boneId = boneNameToIdMap_->at(boneName);
        }

        return boneId;
    }

    const BoneNameToIdMap* GetBoneNameToIdMap() const
    {
        return boneNameToIdMap_;
    }

    Armature* GetArmature()
    {
        return armature_;
    }

    Bone* GetBone(unsigned int index)
    {
        return bones_[index];
    }

    void GetBoneTransforms(std::vector<Matrix>& transforms, const SkeletalAnimation* skeletalAnimation, float time, std::unordered_map<std::string, SocketComponent*>& socketMap);

    void AddSkeletalAnimation(SkeletalAnimation* skeletalAnimation)
    {
        skeletalAnimations_.push_back(skeletalAnimation);
        if (nameToSkeletalAnimationMap_.find(skeletalAnimation->name) == nameToSkeletalAnimationMap_.end())
        {
            nameToSkeletalAnimationMap_[skeletalAnimation->name] = skeletalAnimation;
        }
    }

    const SkeletalAnimation* GetSkeletalAnimation(const std::string& name)
    {
        if (nameToSkeletalAnimationMap_.find(name) != nameToSkeletalAnimationMap_.end())
        {
            return nameToSkeletalAnimationMap_[name];
        }

        return nullptr;
    }

    const NameToSkeletalAnimationMap& GetAnimationsMap() const
    {
        return nameToSkeletalAnimationMap_;
    }

private:
    void SetupTransforms(Bone* bone, const Matrix& parentTransform, std::vector<Matrix>& transforms, const SkeletalAnimation* skeletalAnimation, float time, std::unordered_map<std::string, SocketComponent*>& socketMap);

    BoneNameToIdMap* boneNameToIdMap_{ new BoneNameToIdMap() };

    SkeletalAnimationVector skeletalAnimations_;
    NameToSkeletalAnimationMap nameToSkeletalAnimationMap_;

    std::vector<Bone*> bones_;
    Armature* armature_{ new Armature() };

    unsigned int boneNameToIdMapSize_{ 0 };
    unsigned int boneSize_{ 0 };
};

#endif