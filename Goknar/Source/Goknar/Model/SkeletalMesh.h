#ifndef __SKELETALMESH_H__
#define __SKELETALMESH_H__

#include <string>
#include <unordered_map>
#include <vector>

#include "StaticMesh.h"

struct VertexWeight
{
    unsigned int vertexId;
    float weight;

    VertexWeight() : vertexId(0), weight(0.0f) {}
    VertexWeight(unsigned int pID, float pWeight) : vertexId(pID), weight(pWeight) {}

    bool operator==(const VertexWeight& rhs) const
    {
        return (vertexId == rhs.vertexId && weight == rhs.weight);
    }

    bool operator!=(const VertexWeight& rhs) const
    {
        return (*this == rhs);
    }
};

struct GOKNAR_API SkeletalMeshBoneNode
{

};

struct GOKNAR_API SkeletalMeshBone
{
    SkeletalMeshBone() :
        name(),
        weightsSize(0),
        //#ifndef ASSIMP_BUILD_NO_ARMATUREPOPULATE_PROCESS
        //        mArmature(nullptr),
        //        mNode(nullptr),
        //#endif
        weights(nullptr),
        offsetMatrix() {}

    SkeletalMeshBone(const SkeletalMeshBone& other) :
        name(other.name),
        weightsSize(other.weightsSize),
        //#ifndef ASSIMP_BUILD_NO_ARMATUREPOPULATE_PROCESS
        //        mArmature(nullptr),
        //        mNode(nullptr),
        //#endif
        weights(nullptr),
        offsetMatrix(other.offsetMatrix)
    {
        CopyVertexWeights(other);
    }

    ~SkeletalMeshBone()
    {
        delete[] weights;
    }

	std::string name;

    unsigned int weightsSize;
    VertexWeight* weights;
    Matrix offsetMatrix;

//#ifndef ASSIMP_BUILD_NO_ARMATUREPOPULATE_PROCESS
//    /// The bone armature node - used for skeleton conversion
//    /// you must enable aiProcess_PopulateArmatureData to populate this
//    C_STRUCT aiNode* mArmature;
//    C_STRUCT aiNode* mNode;
//
//#endif

    SkeletalMeshBone& operator=(const SkeletalMeshBone& other)
    {
        if (this == &other)
        {
            return *this;
        }

        name = other.name;
        weightsSize = other.weightsSize;
        offsetMatrix = other.offsetMatrix;
        CopyVertexWeights(other);

        return *this;
    }

    bool operator==(const SkeletalMeshBone& rhs) const
    {
        if (name != rhs.name || weightsSize != rhs.weightsSize)
        {
            return false;
        }

        for (size_t i = 0; i < weightsSize; ++i)
        {
            if (weights[i] != rhs.weights[i])
            {
                return false;
            }
        }

        return true;
    }

    void CopyVertexWeights(const SkeletalMeshBone& other)
    {
        if (other.weights == nullptr || other.weightsSize == 0)
        {
            weights = nullptr;
            weightsSize = 0;
            return;
        }

        weightsSize = other.weightsSize;
        if (weights)
        {
            delete[] weights;
        }

        weights = new VertexWeight[weightsSize];
        ::memcpy(weights, other.weights, weightsSize * sizeof(VertexWeight));
    }
};

#define MAX_BONE_SIZE_PER_VERTEX 4
struct GOKNAR_API VertexToBoneData
{
    VertexToBoneData() : currentIndex(0) {}

    unsigned int boneIDs[MAX_BONE_SIZE_PER_VERTEX] = { 0 };
    float weights[MAX_BONE_SIZE_PER_VERTEX] = { 0.f };

    void AddBoneData(unsigned int id, float weight)
    {
        if (MAX_BONE_SIZE_PER_VERTEX == currentIndex)
        {
            return;
        }

        boneIDs[currentIndex] = id;
        weights[currentIndex] = weight;
        ++currentIndex;
    }

    int currentIndex = 0;
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

    unsigned int boneNameToIdMapSize;
};

#endif