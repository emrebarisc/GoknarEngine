#include "pch.h"

#include "SkeletalMesh.h"

SkeletalMesh::SkeletalMesh() :
	StaticMesh(),
	vertexBoneDataArray_(new VertexBoneDataArray()),
	boneNameToIdMap_(new BoneNameToIdMap()),
	armature_(new Armature()),
	boneNameToIdMapSize_(0),
	boneSize_(0)
{
}

SkeletalMesh::~SkeletalMesh()
{
	int skeletalAnimationSize = skeletalAnimations_.size();
	for (unsigned int skeletalAnimationIndex = 0; skeletalAnimationIndex < skeletalAnimationSize; ++skeletalAnimationIndex)
	{
		delete skeletalAnimations_[skeletalAnimationIndex];
	}

	delete vertexBoneDataArray_;
	delete boneNameToIdMap_;
	delete armature_;
}

void SkeletalMesh::Init()
{
	StaticMesh::Init();
}

void SkeletalMesh::GetBoneTransforms(std::vector<Matrix>& transforms, const SkeletalAnimation* skeletalAnimation, float time)
{
	transforms.resize(boneSize_, Matrix::IdentityMatrix);
	SetupTransforms(armature_->root, Matrix::IdentityMatrix, transforms, skeletalAnimation, time);
}

void SkeletalMesh::SetupTransforms(Bone* bone, const Matrix& parentTransform, std::vector<Matrix>& transforms, const SkeletalAnimation* skeletalAnimation, float time)
{
	if (!bone)
	{
		return;
	}

	Matrix boneTransformation = bone->transformation;
	if (skeletalAnimation)
	{
		SkeletalAnimationNode* skeletalAnimationNode = skeletalAnimation->affectedBoneNameToSkeletalAnimationNodeMap.at(bone->name);

		if (skeletalAnimationNode)
		{
			boneTransformation = skeletalAnimationNode->GetInterpolatedPositionMatrix(time);
			boneTransformation *= skeletalAnimationNode->GetInterpolatedRotationMatrix(time);
			boneTransformation *= skeletalAnimationNode->GetInterpolatedScalingMatrix(time);
		}
	}

	Matrix globalTransformation = parentTransform * boneTransformation;

	transforms[(*boneNameToIdMap_)[bone->name]] = /*armature_->globalInverseTransform * */globalTransformation * bone->offset;
	unsigned int childrenSize = bone->children.size();
	for (unsigned int childIndex = 0; childIndex < childrenSize; ++childIndex)
	{
		SetupTransforms(bone->children[childIndex], globalTransformation, transforms, skeletalAnimation, time);
	}
}
