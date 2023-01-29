#include "pch.h"

#include "SkeletalMesh.h"

SkeletalMesh::SkeletalMesh() :
	StaticMesh(),
	boneNameToIdMapSize(0),
	vertexToBonesArray_(new VertexToBoneDataArray()),
	boneNameToIdMap_(new BoneNameToIdMap())
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
