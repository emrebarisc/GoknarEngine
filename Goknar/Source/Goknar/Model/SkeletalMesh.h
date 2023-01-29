#ifndef __SKELETALMESH_H__
#define __SKELETALMESH_H__

#include <string>
#include <unordered_map>
#include <vector>

#include "StaticMesh.h"
#include "Goknar/Core.h"
#include "Goknar/Log.h"

#define MAX_BONE_SIZE_PER_VERTEX 4

// ANOTHER VARIABLE CANNOT BE ADDED WITHOUT MODIFYING THE RENDERER
struct GOKNAR_API VertexToBoneData
{
    unsigned int boneIDs[MAX_BONE_SIZE_PER_VERTEX] = { 0 };
    float weights[MAX_BONE_SIZE_PER_VERTEX] = { 0.f };

    void AddBoneData(unsigned int id, float weight)
    {
        for (unsigned int i = 0; i < MAX_BONE_SIZE_PER_VERTEX; i++)
        {
            if (weights[i] == 0.0)
            {
                boneIDs[i] = id;
                weights[i] = weight;
                return;
            }
        }

        GOKNAR_CORE_ASSERT(false, "Bone index size cannot be greater than " + std::to_string(MAX_BONE_SIZE_PER_VERTEX));
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