#ifndef __SKELETALMESH_H__
#define __SKELETALMESH_H__

#include <string>
#include <unordered_map>
#include <vector>

#include "StaticMesh.h"
#include "Goknar/Core.h"
#include "Goknar/Log.h"
#include "Goknar/GoknarMath.h"

// TEMP
#include "assimp/matrix3x3.h"
#include "assimp/quaternion.h"

struct GOKNAR_API Bone
{
    Bone() : name(""), offset(Matrix::ZeroMatrix), transformation(Matrix::ZeroMatrix) {}
    Bone(const std::string& n) : name(n), offset(Matrix::ZeroMatrix), transformation(Matrix::ZeroMatrix) {}
    Bone(const std::string& n, const Matrix& o) : name(n), offset(o), transformation(Matrix::ZeroMatrix) {}
    Bone(const std::string& n, const Matrix& o, const Matrix& t) : name(n), offset(o), transformation(t) {}
    Bone(const Bone& rhs) : offset(rhs.offset), transformation(rhs.transformation) {}

    std::string name;

    Matrix offset;
    Matrix transformation;

    std::vector<Bone*> children;
};

struct GOKNAR_API Armature
{
    Armature() : root(nullptr) {}

    Bone* root;
    Matrix globalInverseTransform;
};

// THIS CLASS IS DIRECTLY SENT TO THE GPU
// BE CAUTIOUS OF ADDING OR UPDATING DATA
//
// MAX_BONE_SIZE_PER_VERTEX CANNOT EXCEED 4 
// Since glVertexAttribPointer does not permit sizes more than 4
#define MAX_BONE_SIZE_PER_VERTEX 4
struct GOKNAR_API VertexBoneData
{
    unsigned int boneIDs[MAX_BONE_SIZE_PER_VERTEX] = { 0 };
    float weights[MAX_BONE_SIZE_PER_VERTEX] = { 0.f };

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
};

struct GOKNAR_API AnimationVectorKey
{
    float time;
    Vector3 value;

    AnimationVectorKey() :
        time(0.f),
        value(Vector3::ZeroVector)
    {}

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
};

struct GOKNAR_API AnimationQuaternionKey
{
    double time;
    aiQuaternion value;

    AnimationQuaternionKey() :
        time(0.0),
        value()
    {}

    AnimationQuaternionKey(double t, const aiQuaternion& v) :
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
};

struct GOKNAR_API SkeletalAnimationNode
{
    SkeletalAnimationNode() : 
        rotationKeys(nullptr),
        positionKeys(nullptr),
        scalingKeys(nullptr),
        rotationKeySize(0),
        positionKeySize(0),
        scalingKeySize(0)
    {}

    ~SkeletalAnimationNode()
    {
        delete[] rotationKeys;
        delete[] positionKeys;
        delete[] scalingKeys;
    }

    Matrix GetInterpolatedScalingMatrix(float time)
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
        
        return Matrix::GetScalingMatrix(GoknarMath::LinearInterpolation(scalingKeys[previousIndex].value, scalingKeys[nextIndex].value, alpha));
    }

    Matrix GetInterpolatedPositionMatrix(float time)
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

        return Matrix::GetPositionMatrix(GoknarMath::LinearInterpolation(positionKeys[previousIndex].value, positionKeys[nextIndex].value, alpha));
    }

    Matrix GetInterpolatedRotationMatrix(float time)
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

        const aiQuaternion& startRotation = rotationKeys[previousIndex].value;
        const aiQuaternion& endRotation = rotationKeys[nextIndex].value;
        aiQuaternion out;
        aiQuaternion::Interpolate(out, startRotation, endRotation, alpha);
        out.Normalize();

        aiMatrix3x3 assimpMatrix = out.GetMatrix();

        return Matrix(  assimpMatrix.a1, assimpMatrix.a2, assimpMatrix.a3, 0.f,
                        assimpMatrix.b1, assimpMatrix.b2, assimpMatrix.b3, 0.f,
                        assimpMatrix.c1, assimpMatrix.c2, assimpMatrix.c3, 0.f,
                        0.f, 0.f, 0.f, 1.f);
    }

    std::string affectedBoneName;

    AnimationQuaternionKey* rotationKeys;
    AnimationVectorKey* positionKeys;
    AnimationVectorKey* scalingKeys;

    int rotationKeySize;
    int positionKeySize;
    int scalingKeySize;
};

struct GOKNAR_API SkeletalAnimation
{
    SkeletalAnimation() :
        animationNodes(nullptr),
        name(""),
        duration(0.f),
        ticksPerSecond(0.f),
        animationNodeSize(0)
    {}

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
    SkeletalAnimationNode** animationNodes;
    std::string name;
    float duration;
    float ticksPerSecond;
    unsigned int animationNodeSize;
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

    void GetBoneTransforms(std::vector<Matrix>& transforms, const SkeletalAnimation* skeletalAnimation, float time);

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
    void SetupTransforms(Bone* bone, const Matrix& parentTransform, std::vector<Matrix>& transforms, const SkeletalAnimation* skeletalAnimation, float time);

    VertexBoneDataArray* vertexBoneDataArray_;
    BoneNameToIdMap* boneNameToIdMap_;

    SkeletalAnimationVector skeletalAnimations_;
    NameToSkeletalAnimationMap nameToSkeletalAnimationMap_;

    std::vector<Bone*> bones_;
    Armature* armature_;

    unsigned int boneNameToIdMapSize_;
    unsigned int boneSize_;
};

#endif