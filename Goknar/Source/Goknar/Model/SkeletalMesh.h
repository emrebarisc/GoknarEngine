#ifndef __SKELETALMESH_H__
#define __SKELETALMESH_H__

#include <string>
#include <unordered_map>
#include <vector>

#include "StaticMesh.h"
#include "Goknar/Core.h"
#include "Goknar/Log.h"
#include "Goknar/GoknarMath.h"

#define MAX_BONE_SIZE_PER_VERTEX 4

struct GOKNAR_API Bone
{
    Bone() : offset(Matrix::IdentityMatrix), transformation(Matrix::IdentityMatrix) {}
    Bone(const Matrix& o) : offset(o), transformation(Matrix::IdentityMatrix) {}
    Bone(const Matrix& o, const Matrix& t) : offset(o), transformation(t) {}
    Bone(const Bone& rhs) : offset(rhs.offset), transformation(rhs.transformation) {}

    Matrix offset;
    Matrix transformation;

    std::vector<Bone*> children;
};

struct GOKNAR_API Armature
{
    Armature() : root(nullptr) {}

    Bone* root;
};

// ANOTHER VARIABLE CANNOT BE ADDED WITHOUT MODIFYING THE RENDERER
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
            if (boneIDs[i] == id)
            {
                return;
            }

            // TODO_Baris: Optimize
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

typedef std::vector<VertexBoneData> VertexBoneDataArray;
typedef std::unordered_map<std::string, unsigned int> BoneNameToIdMap;

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

    void AddBone(const Matrix& offset)
    {
        bones_.emplace_back(new Bone(offset));
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
            boneId = boneNameToIdMapSize;
            (*boneNameToIdMap_)[boneName] = boneId;
            ++boneNameToIdMapSize;
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
        return armature;
    }

    Bone* GetBone(unsigned int index)
    {
        return bones_[index];
    }

    void GetBoneTransforms(std::vector<Matrix>& transforms);

private:
    void SetupTransforms(Bone* bone, const Matrix& parentTransform);

    VertexBoneDataArray* vertexBoneDataArray_;
    BoneNameToIdMap* boneNameToIdMap_;

    std::vector<Bone*> bones_;
    Armature* armature;

    unsigned int boneNameToIdMapSize;
    unsigned int boneSize_;
};

#endif