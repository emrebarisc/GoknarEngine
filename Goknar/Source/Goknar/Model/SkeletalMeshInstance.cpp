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
	auto& animation = skeletalMeshAnimation_;
	if (!animation.skeletalAnimation)
	{
		return;
	}

	const float durationInSeconds = animation.skeletalAnimation->duration;
	const float fps = animation.skeletalAnimation->ticksPerSecond;

	const float newElapsedTime = engine->GetElapsedTime() - animation.initialTimeInSeconds;

	float newAnimationTime = std::fmod(newElapsedTime, durationInSeconds);

	const bool loopedThisFrame = newAnimationTime < animation.animationTime;

	if (loopedThisFrame && animation.playLoopData.playOnce)
	{
		if (!animation.playLoopData.callback.isNull())
		{
			animation.playLoopData.callback();
		}
		return;
	}

	animation.elapsedTimeInSeconds = newElapsedTime;
	float oldAnimationTime = animation.animationTime;
	animation.animationTime = newAnimationTime;

	if (loopedThisFrame && !animation.playLoopData.callback.isNull())
	{
		animation.playLoopData.callback();
	}

	int startFrameIndex = (int)(oldAnimationTime * fps);
	int endFrameIndex = (int)(newAnimationTime * fps);

	if (startFrameIndex != endFrameIndex)
	{
		auto& callbackMap = animation.keyframeData.keyframeCallbackMap;

		std::function<void(int, int)> triggerRange =
			[&](int from, int to)
			{
				for (int i = from; i <= to; ++i)
				{
					auto it = callbackMap.find(i);
					if (it != callbackMap.end())
					{
						it->second();
					}
				}
			};

		if (loopedThisFrame)
		{
			int maxFrames = (int)(durationInSeconds * fps);
			triggerRange(startFrameIndex + 1, maxFrames);
			triggerRange(0, endFrameIndex);
		}
		else
		{
			triggerRange(startFrameIndex + 1, endFrameIndex);
		}

		animation.currentKeyframe = endFrameIndex;
	}
}

void SkeletalMeshInstance::Render(RenderPassType renderPassType)
{
	SetRenderOperations(renderPassType);
}

void SkeletalMeshInstance::SetRenderOperations(RenderPassType renderPassType)
{
	for (SkeletalMeshUnit* subMesh : mesh_->GetSubMeshes())
	{
		subMesh->GetMaterial()->GetShader(renderPassType)->SetMatrixVector(SHADER_VARIABLE_NAMES::SKELETAL_MESH::BONES, boneTransformations_);
	}
	IMeshInstance::Render(renderPassType);
}

void SkeletalMeshInstance::SetMesh(SkeletalMesh* skeletalMesh)
{
	IMeshInstance::SetMesh(skeletalMesh);

	boneTransformations_.resize(skeletalMesh->GetBoneSize(), Matrix::IdentityMatrix);
}

void SkeletalMeshInstance::PlayAnimation(const std::string& animationName, const PlayLoopData& playLoopData/* = { false, {} }*/, const KeyframeData& keyframeData/* = {}*/)
{
	if (animationName.empty() ||
		(skeletalMeshAnimation_.skeletalAnimation &&
		skeletalMeshAnimation_.skeletalAnimation->name == animationName))
	{
		return;
	}

	SkeletalMesh* skeletalMesh = GetMesh();
	if (!skeletalMesh)
	{
		return;
	}

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
