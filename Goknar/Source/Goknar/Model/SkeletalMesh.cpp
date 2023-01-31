#include "pch.h"

#include "SkeletalMesh.h"

SkeletalMesh::SkeletalMesh() :
	StaticMesh(),
	vertexBoneDataArray_(new VertexBoneDataArray()),
	boneNameToIdMap_(new BoneNameToIdMap()),
	armature(new Armature()),
	boneNameToIdMapSize(0),
	boneSize_(0)
{
}

SkeletalMesh::~SkeletalMesh()
{
	delete vertexBoneDataArray_;
	delete boneNameToIdMap_;
	delete armature;
}

void SkeletalMesh::Init()
{
	StaticMesh::Init();
}

void SkeletalMesh::GetBoneTransforms(std::vector<Matrix>& transforms)
{
	transforms.resize(boneSize_, Matrix::IdentityMatrix);

	//static bool calculated = false;
	//if (!calculated)
	//{
	//	calculated = true;
	//	SetupTransforms(armature->root, Matrix::IdentityMatrix);
	//}
	for(unsigned int boneIndex = 0; boneIndex < boneSize_; ++boneIndex)
	{
		transforms[boneIndex] = bones_[boneIndex]->transformation;
	}
}

void SkeletalMesh::SetupTransforms(Bone* bone, const Matrix& parentTransform)
{
	Matrix boneTransformation = bone->transformation;
	Matrix globalTransformation = parentTransform * boneTransformation;

	bone->transformation = globalTransformation * bone->offset;

	unsigned int childrenSize = bone->children.size();
	for (unsigned int childIndex = 0; childIndex < childrenSize; ++childIndex)
	{
		SetupTransforms(bone->children[childIndex], globalTransformation);
	}
}
