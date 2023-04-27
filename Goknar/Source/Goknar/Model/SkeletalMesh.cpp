#include "pch.h"

#include "SkeletalMesh.h"

#include "Goknar/Engine.h"
#include "Goknar/Components/SocketComponent.h"

SkeletalMesh::SkeletalMesh() :
	StaticMesh()
{
}

SkeletalMesh::~SkeletalMesh()
{
	int skeletalAnimationSize = skeletalAnimations_.size();
	for (unsigned int skeletalAnimationIndex = 0; skeletalAnimationIndex < skeletalAnimationSize; ++skeletalAnimationIndex)
	{
		delete skeletalAnimations_[skeletalAnimationIndex];
	}

	int bonesSize = bones_.size();
	for (unsigned int boneIndex = 0; boneIndex < bonesSize; ++boneIndex)
	{
		delete bones_[boneIndex];
	}

	delete vertexBoneDataArray_;
	delete boneNameToIdMap_;
	delete armature_;
}

void SkeletalMesh::Init()
{
	MeshUnit::Init();

	engine->AddSkeletalMeshToRenderer(this);
}

void SkeletalMesh::GetBoneTransforms(std::vector<Matrix>& transforms, const SkeletalAnimation* skeletalAnimation, float time, std::unordered_map<std::string, SocketComponent*>& socketMap)
{
	SetupTransforms(armature_->root, Matrix::IdentityMatrix, transforms, skeletalAnimation, time, socketMap);
}

void SkeletalMesh::SetupTransforms(Bone* bone, const Matrix& parentTransform, std::vector<Matrix>& transforms, const SkeletalAnimation* skeletalAnimation, float time, std::unordered_map<std::string, SocketComponent*>& socketMap)
{
	if (!bone)
	{
		return;
	}

	Vector3 interpolatedPosition = Vector3::ZeroVector;
	Vector3 interpolatedScaling = Vector3(1.f);
	Quaternion interpolatedRotation = Quaternion::Identity;

	Matrix boneTransformation = bone->transformation;
	if (skeletalAnimation)
	{
		SkeletalAnimationNode* skeletalAnimationNode = skeletalAnimation->affectedBoneNameToSkeletalAnimationNodeMap.at(bone->name);

		if (skeletalAnimationNode)
		{
			skeletalAnimationNode->GetInterpolatedPosition(interpolatedPosition, time);
			skeletalAnimationNode->GetInterpolatedRotation(interpolatedRotation, time);
			skeletalAnimationNode->GetInterpolatedScaling(interpolatedScaling, time);

			boneTransformation = Matrix::GetTransformationMatrix(interpolatedRotation, interpolatedPosition, interpolatedScaling);
		}
	}

	Matrix globalTransformation = parentTransform * boneTransformation;

	Matrix transform = /*armature_->globalInverseTransform * */globalTransformation * bone->offset;

	if (socketMap.find(bone->name) != socketMap.end())
	{
		socketMap[bone->name]->SetBoneTransformationMatrix(globalTransformation);
	}

	transforms[(*boneNameToIdMap_)[bone->name]] = transform;
	unsigned int childrenSize = bone->children.size();
	for (unsigned int childIndex = 0; childIndex < childrenSize; ++childIndex)
	{
		SetupTransforms(bone->children[childIndex], globalTransformation, transforms, skeletalAnimation, time, socketMap);
	}
}
