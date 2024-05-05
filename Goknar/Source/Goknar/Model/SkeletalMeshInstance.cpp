#include "pch.h"

#include "SkeletalMeshInstance.h"

#include <algorithm>
#include <execution>

#include "Goknar/Engine.h"
#include "Goknar/Components/SocketComponent.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/ShaderTypes.h"

SkeletalMeshInstance::SkeletalMeshInstance(RenderComponent* parentComponent) :
	IMeshInstance(parentComponent)
{
}

SkeletalMeshInstance::~SkeletalMeshInstance()
{
}

void SkeletalMeshInstance::PrepareForTheCurrentFrame()
{
	mesh_->GetBoneTransforms(boneTransformations_, skeletalMeshAnimation_.skeletalAnimation, skeletalMeshAnimation_.animationTime, sockets_);

	// TODO: Implement a proper multi threading
	// Following std::for_each parallelizing causes game crash
	//std::for_each
	//(
	//	std::execution::par,
	//	std::begin(boneIdToAttachedMatrixPointerMap_),
	//	std::end(boneIdToAttachedMatrixPointerMap_),
	//	[&](const std::pair<int, const Matrix*>& pair)
	//	{
	//		const Matrix* matrix = pair.second;

	//		if (matrix)
	//		{
	//			boneTransformations_[pair.first] = parentComponent_->GetComponentToWorldTransformationMatrix().GetInverse() * *matrix;
	//		}
	//	}
	//);

	std::unordered_map<int, const Matrix*>::iterator boneIdToAttachedMatrixPointerMapIterator = boneIdToAttachedMatrixPointerMap_.begin();
	while (boneIdToAttachedMatrixPointerMapIterator != boneIdToAttachedMatrixPointerMap_.end())
	{
		const Matrix* matrix = boneIdToAttachedMatrixPointerMapIterator->second;

		if (matrix)
		{
			boneTransformations_[boneIdToAttachedMatrixPointerMapIterator->first] = parentComponent_->GetComponentToWorldTransformationMatrix().GetInverse() * *matrix;
		}

		++boneIdToAttachedMatrixPointerMapIterator;
	}
}

void SkeletalMeshInstance::PrepareForTheNextFrame()
{
	if (skeletalMeshAnimation_.skeletalAnimation)
	{
		const float newElapsedTimeInSeconds = engine->GetElapsedTime() - skeletalMeshAnimation_.initialTimeInSeconds;

		const float newKeyframeIndex = skeletalMeshAnimation_.skeletalAnimation->ticksPerSecond * newElapsedTimeInSeconds;

		const float floorQuotient = std::floor(newKeyframeIndex / skeletalMeshAnimation_.skeletalAnimation->duration);
		const float normalizedNewKeyframeIndex = newKeyframeIndex - (floorQuotient * skeletalMeshAnimation_.skeletalAnimation->duration);

		const bool isAnimationLoopedThisFrame = normalizedNewKeyframeIndex < skeletalMeshAnimation_.animationTime;

		if (!isAnimationLoopedThisFrame ||
			(isAnimationLoopedThisFrame && !skeletalMeshAnimation_.playLoopData.playOnce))
		{
			skeletalMeshAnimation_.elapsedTimeInSeconds = newElapsedTimeInSeconds;
			skeletalMeshAnimation_.animationTime = normalizedNewKeyframeIndex;

			if (isAnimationLoopedThisFrame && skeletalMeshAnimation_.playLoopData.callback != nullptr)
			{
				skeletalMeshAnimation_.playLoopData.callback();
			}
		}
		else
		{
			if (skeletalMeshAnimation_.playLoopData.callback != nullptr)
			{
				skeletalMeshAnimation_.playLoopData.callback();
			}
		}

		// TODO: OPTIMIZE
		const int currentKeyframe = (int)std::floor(skeletalMeshAnimation_.animationTime);
		if (currentKeyframe != skeletalMeshAnimation_.currentKeyframe)
		{
			decltype(skeletalMeshAnimation_.keyframeData.keyframeCallbackMap)& keyframeCallbackMap = skeletalMeshAnimation_.keyframeData.keyframeCallbackMap;
			
			int keyframeIndex = skeletalMeshAnimation_.currentKeyframe;
			while(keyframeIndex < currentKeyframe)
			{
				if (keyframeCallbackMap.find(keyframeIndex) != keyframeCallbackMap.end())
				{
					keyframeCallbackMap[keyframeIndex]();
				}

				keyframeIndex = (keyframeIndex + 1) % skeletalMeshAnimation_.skeletalAnimation->maxKeyframe;
			}
			
			skeletalMeshAnimation_.currentKeyframe = currentKeyframe;
		}
	}

	// mesh_->GetBoneTransforms(boneTransformations_, skeletalMeshAnimation_.skeletalAnimation, skeletalMeshAnimation_.animationTime, sockets_);

	// // TODO: Implement a proper multi threading
	// // Following std::for_each parallelizing causes game crash
	// //std::for_each
	// //(
	// //	std::execution::par,
	// //	std::begin(boneIdToAttachedMatrixPointerMap_),
	// //	std::end(boneIdToAttachedMatrixPointerMap_),
	// //	[&](const std::pair<int, const Matrix*>& pair)
	// //	{
	// //		const Matrix* matrix = pair.second;

	// //		if (matrix)
	// //		{
	// //			boneTransformations_[pair.first] = parentComponent_->GetComponentToWorldTransformationMatrix().GetInverse() * *matrix;
	// //		}
	// //	}
	// //);

	// std::unordered_map<int, const Matrix*>::iterator boneIdToAttachedMatrixPointerMapIterator = boneIdToAttachedMatrixPointerMap_.begin();
	// while (boneIdToAttachedMatrixPointerMapIterator != boneIdToAttachedMatrixPointerMap_.end())
	// {
	// 	const Matrix* matrix = boneIdToAttachedMatrixPointerMapIterator->second;

	// 	if (matrix)
	// 	{
	// 		boneTransformations_[boneIdToAttachedMatrixPointerMapIterator->first] = parentComponent_->GetComponentToWorldTransformationMatrix().GetInverse() * *matrix;
	// 	}

	// 	++boneIdToAttachedMatrixPointerMapIterator;
	// }
}

void SkeletalMeshInstance::Render(RenderPassType renderPassType)
{
	PreRender(renderPassType);

	SetRenderOperations(renderPassType);
}

void SkeletalMeshInstance::SetRenderOperations(RenderPassType renderPassType)
{
	mesh_->GetMaterial()->GetShader(renderPassType)->SetMatrixVector(SHADER_VARIABLE_NAMES::SKELETAL_MESH::BONES, boneTransformations_);
	IMeshInstance::Render(renderPassType);
}

void SkeletalMeshInstance::SetMesh(SkeletalMesh* skeletalMesh)
{
	IMeshInstance::SetMesh(skeletalMesh);

	boneTransformations_.resize(skeletalMesh->GetBoneSize(), Matrix::IdentityMatrix);
}

void SkeletalMeshInstance::PlayAnimation(const std::string& animationName, const PlayLoopData& playLoopData/* = { false, {} }*/, const KeyframeData& keyframeData/* = {}*/)
{
	if (!skeletalMeshAnimation_.skeletalAnimation || 
		skeletalMeshAnimation_.skeletalAnimation && skeletalMeshAnimation_.skeletalAnimation->name != animationName)
	{
		SkeletalMesh* skeletalMesh = GetMesh();
		skeletalMeshAnimation_.skeletalAnimation = skeletalMesh->GetSkeletalAnimation(animationName);
		if (!skeletalMeshAnimation_.skeletalAnimation)
		{
			return;
		}
		skeletalMeshAnimation_.animationTime = 0.f;
		skeletalMeshAnimation_.elapsedTimeInSeconds = 0.f;
		skeletalMeshAnimation_.initialTimeInSeconds = engine->GetElapsedTime();

		skeletalMeshAnimation_.playLoopData = playLoopData;
		skeletalMeshAnimation_.keyframeData = keyframeData;
	}
}

void SkeletalMeshInstance::AttachBoneToMatrixPointer(const std::string& boneName, const Matrix* matrix)
{
	const int boneId = GetMesh()->GetBoneId(boneName);

	if (matrix)
	{
		boneIdToAttachedMatrixPointerMap_[boneId] = matrix;
	}
	else
	{
		decltype(boneIdToAttachedMatrixPointerMap_)::iterator iterator = boneIdToAttachedMatrixPointerMap_.find(boneId);

		if (iterator == boneIdToAttachedMatrixPointerMap_.end())
		{
			return;
		}

		boneIdToAttachedMatrixPointerMap_.erase(iterator);
	}
}

void SkeletalMeshInstance::RemoveBoneToMatrixPointer(const std::string& boneName)
{
	const int boneId = GetMesh()->GetBoneId(boneName);
	decltype(boneIdToAttachedMatrixPointerMap_)::iterator iterator = boneIdToAttachedMatrixPointerMap_.find(boneId);
	if (iterator != boneIdToAttachedMatrixPointerMap_.end())
	{
		boneIdToAttachedMatrixPointerMap_.erase(iterator);
	}
}

void SkeletalMeshInstance::AddMeshInstanceToRenderer()
{
	engine->GetRenderer()->AddSkeletalMeshInstance(this);
}

void SkeletalMeshInstance::RemoveMeshInstanceFromRenderer()
{
	engine->GetRenderer()->RemoveSkeletalMeshInstance(this);
}

SocketComponent* SkeletalMeshInstance::AddSocketToBone(const std::string& boneName)
{
	if (sockets_.find(boneName) == sockets_.end())
	{
		SocketComponent* socketComponent = new SocketComponent(parentComponent_);
		socketComponent->SetOwner(parentComponent_->GetOwner());
		sockets_[boneName] = socketComponent;
	}

	return sockets_[boneName];
}

SocketComponent* SkeletalMeshInstance::GetSocket(const std::string& boneName)
{
	if (sockets_.find(boneName) != sockets_.end())
	{
		return sockets_[boneName];
	}

	return nullptr;
}
