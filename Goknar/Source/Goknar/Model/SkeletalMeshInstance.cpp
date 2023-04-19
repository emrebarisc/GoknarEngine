#include "pch.h"

#include "SkeletalMeshInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Components/SocketComponent.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/ShaderTypes.h"

SkeletalMeshInstance::SkeletalMeshInstance(RenderComponent* parentComponent) :
	MeshInstance(parentComponent)
{
}

SkeletalMeshInstance::~SkeletalMeshInstance()
{
}

void SkeletalMeshInstance::Render()
{
	PreRender();

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

		const int currentKeyframe = (int)std::floor(skeletalMeshAnimation_.animationTime);
		if (currentKeyframe != skeletalMeshAnimation_.currentKeyframe)
		{
			skeletalMeshAnimation_.currentKeyframe = currentKeyframe;
			if (skeletalMeshAnimation_.keyframeData.keyframeCallbackMap.find(currentKeyframe) != skeletalMeshAnimation_.keyframeData.keyframeCallbackMap.end())
			{
				skeletalMeshAnimation_.keyframeData.keyframeCallbackMap[currentKeyframe]();
			}
		}
	}

	mesh_->GetBoneTransforms(boneTransformations_, skeletalMeshAnimation_.skeletalAnimation, skeletalMeshAnimation_.animationTime, sockets_);
	mesh_->GetMaterial()->GetShader()->SetMatrixVector(SHADER_VARIABLE_NAMES::SKELETAL_MESH::BONES, boneTransformations_);

	MeshInstance::Render();
}

void SkeletalMeshInstance::SetMesh(SkeletalMesh* skeletalMesh)
{
	MeshInstance::SetMesh(skeletalMesh);

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
