#include "pch.h"

#include "SkeletalMesh.h"

SkeletalMesh::SkeletalMesh() :
	StaticMesh(),
	vertexToBonesArray_(new VertexToBoneDataArray()),
	boneNameToIdMap_(new BoneNameToIdMap()),
	boneNameToIdMapSize(0),
	boneSize_(0)
{
}

SkeletalMesh::~SkeletalMesh()
{
	delete vertexToBonesArray_;
	delete boneNameToIdMap_;
}

void SkeletalMesh::Init()
{
	StaticMesh::Init();
}
