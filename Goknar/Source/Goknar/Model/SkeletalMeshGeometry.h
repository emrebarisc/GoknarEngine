#ifndef __SKELETALMESHGEOMETRY_H__
#define __SKELETALMESHGEOMETRY_H__

#include "MeshGeometry.h"

#include <vector>

class SkeletalMeshLOD;

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
    }

    void NormalizeWeights()
    {
        float totalWeight = 0.f;
        for (unsigned int i = 0; i < MAX_BONE_SIZE_PER_VERTEX; ++i)
        {
            totalWeight += weights[i];
        }

        if (totalWeight <= 0.f)
        {
            boneIDs[0] = 0;
            weights[0] = 1.f;
            for (unsigned int i = 1; i < MAX_BONE_SIZE_PER_VERTEX; ++i)
            {
                boneIDs[i] = 0;
                weights[i] = 0.f;
            }
            return;
        }

        const float inverseTotalWeight = 1.f / totalWeight;
        for (unsigned int i = 0; i < MAX_BONE_SIZE_PER_VERTEX; ++i)
        {
            weights[i] *= inverseTotalWeight;
        }
    }

    unsigned int boneIDs[MAX_BONE_SIZE_PER_VERTEX] = { 0 };
    float weights[MAX_BONE_SIZE_PER_VERTEX] = { 0.f };
};

typedef std::vector<VertexBoneData> VertexBoneDataArray;

class GOKNAR_API SkeletalMeshGeometry : public MeshGeometry
{
public:
	SkeletalMeshGeometry();
	virtual ~SkeletalMeshGeometry();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;

    void ResizeVertexToBonesArray(unsigned int size)
    {
        vertexBoneDataArray_->resize(size);
    }

    void AddVertexBoneData(unsigned int index, unsigned int id, float weight)
    {
        vertexBoneDataArray_->at(index).AddBoneData(id, weight);
    }

    const VertexBoneDataArray* GetVertexBoneDataArray() const
    {
        return vertexBoneDataArray_;
    }

    void NormalizeVertexBoneWeights()
    {
        for (VertexBoneData& vertexBoneData : *vertexBoneDataArray_)
        {
            vertexBoneData.NormalizeWeights();
        }
    }

    void SetOwner(const SkeletalMeshLOD* const owner)
    {
        owner_ = owner;
    }

    const SkeletalMeshLOD* GetOwner() const
    {
        return owner_;
    }

protected:

private:
    VertexBoneDataArray* vertexBoneDataArray_{ new VertexBoneDataArray() };

    const SkeletalMeshLOD* owner_;
};

#endif
