#ifndef __SKELETALMESHUNIT_H__
#define __SKELETALMESHUNIT_H__

#include "MeshUnit.h"

#include <vector>

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

    unsigned int boneIDs[MAX_BONE_SIZE_PER_VERTEX] = { 0 };
    float weights[MAX_BONE_SIZE_PER_VERTEX] = { 0.f };
};

typedef std::vector<VertexBoneData> VertexBoneDataArray;

class GOKNAR_API SkeletalMeshUnit : public MeshUnit
{
public:
	SkeletalMeshUnit();
	virtual ~SkeletalMeshUnit();

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

protected:

private:
    VertexBoneDataArray* vertexBoneDataArray_{ new VertexBoneDataArray() };
};

#endif