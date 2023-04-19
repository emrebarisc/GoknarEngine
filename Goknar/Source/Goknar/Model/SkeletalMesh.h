#ifndef __SKELETALMESH_H__
#define __SKELETALMESH_H__

#include <string>
#include <unordered_map>
#include <vector>

#include "StaticMesh.h"
#include "Goknar/Core.h"
#include "Goknar/Log.h"
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

// THIS CLASS IS DIRECTLY SENT TO THE GPU
// BE CAUTIOUS OF ADDING OR UPDATING DATA
//
// MAX_BONE_SIZE_PER_VERTEX CANNOT EXCEED 4 
// Since glVertexAttribPointer does not permit sizes more than 4
#define MAX_BONE_SIZE_PER_VERTEX 4
struct GOKNAR_API VertexBoneData
{
    void AddBoneData(unsigned int id, float weight)
    {
        int smallestIndex = -1;
        float largestDifference = 0.f;
        for (unsigned int i = 0; i < MAX_BONE_SIZE_PER_VERTEX; ++i)
        {
            if (boneIDs[i] == id && weights[i] != 0.f)
            {
                return;
            }

            float difference = weight - weights[i];
            if (largestDifference < difference)
            {
                largestDifference = difference;
                smallestIndex = i;
            }
        }

        if (0 <= smallestIndex)
        {
            boneIDs[smallestIndex] = id;
            weights[smallestIndex] = weight;
        }

        //GOKNAR_CORE_ASSERT(false, "Bone index size cannot be greater than " + std::to_string(MAX_BONE_SIZE_PER_VERTEX));
    }

    unsigned int boneIDs[MAX_BONE_SIZE_PER_VERTEX] = { 0 };
    float weights[MAX_BONE_SIZE_PER_VERTEX] = { 0.f };
};

struct GOKNAR_API AnimationVectorKey
{
    AnimationVectorKey() {}

    AnimationVectorKey(double time, const Vector3& value) :
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

    AnimationQuaternionKey(double t, const Quaternion& v) :
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

    double time{ 0.0 };
    Quaternion value{ Quaternion::Identity };
};

struct GOKNAR_API SkeletalAnimationNode
{
    ~SkeletalAnimationNode()
    {
        delete[] rotationKeys;
        delete[] positionKeys;
        delete[] scalingKeys;
    }

    void GetInterpolatedScaling(Vector3& out, float time)
    {
        int previousIndex = 0;
        for (unsigned int scalingIndex = 0; scalingIndex < scalingKeySize; ++scalingIndex)
        {
            if (scalingKeys[scalingIndex].time < time)
            {
                previousIndex = scalingIndex;
            }
        }

        int nextIndex = previousIndex + 1;

        float alpha = (time - scalingKeys[previousIndex].time) / (scalingKeys[nextIndex].time - scalingKeys[previousIndex].time);

        out = GoknarMath::Slerp(scalingKeys[previousIndex].value, scalingKeys[nextIndex].value, alpha);
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
        for (unsigned int positionIndex = 0; positionIndex < positionKeySize; ++positionIndex)
        {
            if (positionKeys[positionIndex].time < time)
            {
                previousIndex = positionIndex;
            }
        }

        int nextIndex = previousIndex + 1;

        float alpha = (time - positionKeys[previousIndex].time) / (positionKeys[nextIndex].time - positionKeys[previousIndex].time);

        out = GoknarMath::Slerp(positionKeys[previousIndex].value, positionKeys[nextIndex].value, alpha);
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
        for (unsigned int positionIndex = 0; positionIndex < rotationKeySize; ++positionIndex)
        {
            if (rotationKeys[positionIndex].time < time)
            {
                previousIndex = positionIndex;
            }
        }

        int nextIndex = previousIndex + 1;

        float alpha = (time - rotationKeys[previousIndex].time) / (rotationKeys[nextIndex].time - rotationKeys[previousIndex].time);

        const Quaternion& startRotation = rotationKeys[previousIndex].value;
        const Quaternion& endRotation = rotationKeys[nextIndex].value;
        out = GoknarMath::Slerp(startRotation, endRotation, alpha);
        out.Normalize();
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
        for (unsigned int animationNodeIndex = 0; animationNodeIndex < animationNodeSize; ++animationNodeIndex)
        {
            delete animationNodes[animationNodeIndex];
        }
        delete[] animationNodes;
    }

    void AddSkeletalAnimationNode(int index, SkeletalAnimationNode* skeletalAnimationNode)
    {
        animationNodes[index] = skeletalAnimationNode;
        affectedBoneNameToSkeletalAnimationNodeMap[skeletalAnimationNode->affectedBoneName] = skeletalAnimationNode;
    }

    std::unordered_map<std::string, SkeletalAnimationNode*> affectedBoneNameToSkeletalAnimationNodeMap;
    SkeletalAnimationNode** animationNodes{ nullptr };
    std::string name{ "" };
    float duration{ 0.f };
    float ticksPerSecond{ 0.f };
    unsigned int animationNodeSize{ 0 };
};

typedef std::vector<VertexBoneData> VertexBoneDataArray;
typedef std::unordered_map<std::string, unsigned int> BoneNameToIdMap;

typedef std::vector<SkeletalAnimation*> SkeletalAnimationVector;
typedef std::unordered_map<std::string, SkeletalAnimation*> NameToSkeletalAnimationMap;

class GOKNAR_API SkeletalMesh : public StaticMesh
{
public:
	SkeletalMesh();

	virtual ~SkeletalMesh();

	virtual void Init();

    void ResizeVertexToBonesArray(unsigned int size)
    {
        vertexBoneDataArray_->resize(size);
    }

    void AddVertexBoneData(unsigned int index, unsigned int id, float weight)
    {
        vertexBoneDataArray_->at(index).AddBoneData(id, weight);
    }

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

    const VertexBoneDataArray* GetVertexBoneDataArray() const
    {
        return vertexBoneDataArray_;
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

private:
    void SetupTransforms(Bone* bone, const Matrix& parentTransform, std::vector<Matrix>& transforms, const SkeletalAnimation* skeletalAnimation, float time, std::unordered_map<std::string, SocketComponent*>& socketMap);

    VertexBoneDataArray* vertexBoneDataArray_{ new VertexBoneDataArray() };
    BoneNameToIdMap* boneNameToIdMap_{ new BoneNameToIdMap() };

    SkeletalAnimationVector skeletalAnimations_;
    NameToSkeletalAnimationMap nameToSkeletalAnimationMap_;

    std::vector<Bone*> bones_;
    Armature* armature_{ new Armature() };

    unsigned int boneNameToIdMapSize_{ 0 };
    unsigned int boneSize_{ 0 };
};

#endif