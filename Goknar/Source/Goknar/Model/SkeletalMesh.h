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
};

// ANOTHER VARIABLE CANNOT BE ADDED WITHOUT MODIFYING THE RENDERER
struct GOKNAR_API VertexToBoneData
{
    unsigned int boneIDs[MAX_BONE_SIZE_PER_VERTEX] = { 0 };
    float weights[MAX_BONE_SIZE_PER_VERTEX] = { 0.f };

    void AddBoneData(unsigned int id, float weight)
    {
        int smallestIndex = -1;
        int largestDifference = 0.f;
        for (unsigned int i = 0; i < MAX_BONE_SIZE_PER_VERTEX; ++i)
        {
            // TODO_Baris: Optimize
            int difference = weight - weights[i];
            if (largestDifference < difference)
            {
                largestDifference = difference;
                smallestIndex = i;
            }

            if (0 <= smallestIndex)
            {
                boneIDs[smallestIndex] = id;
                weights[smallestIndex] = weight;
            }
        }

        //GOKNAR_CORE_ASSERT(false, "Bone index size cannot be greater than " + std::to_string(MAX_BONE_SIZE_PER_VERTEX));
    }
};

typedef std::vector<VertexToBoneData> VertexToBoneDataArray;
typedef std::unordered_map<std::string, unsigned int> BoneNameToIdMap;

class GOKNAR_API SkeletalMesh : public StaticMesh
{
public:
	SkeletalMesh();

	virtual ~SkeletalMesh();

	virtual void Init();

    void ResizeVertexToBonesArray(unsigned int size)
    {
        vertexToBonesArray_->resize(size);
    }

    void AddVertexToBoneData(unsigned int index, unsigned int id, float weight)
    {
        vertexToBonesArray_->at(index).AddBoneData(id, weight);
    }

    void AddBone(const Bone& bone)
    {
        bones_.push_back(bone);
        ++boneSize_;
    }

    void AddBone(const Matrix& offset)
    {
        bones_.emplace_back(offset);
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

    const VertexToBoneDataArray* GetVertexToBonesArray() const
    {
        return vertexToBonesArray_;
    }

    const BoneNameToIdMap* GetBoneNameToIdMap() const
    {
        return boneNameToIdMap_;
    }

private:
    VertexToBoneDataArray* vertexToBonesArray_;
    BoneNameToIdMap* boneNameToIdMap_;

    std::vector<Bone> bones_;

    unsigned int boneNameToIdMapSize;
    unsigned int boneSize_;
};

#endif