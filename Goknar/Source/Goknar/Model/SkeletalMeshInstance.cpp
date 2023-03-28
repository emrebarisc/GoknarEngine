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
	std::unordered_map<std::string, SocketComponent*>::iterator socketComponentIterator = sockets_.begin();
	for (; socketComponentIterator != sockets_.end(); ++socketComponentIterator)
	{
		delete socketComponentIterator->second;
	}
}

void SkeletalMeshInstance::Render()
{
	PreRender();

	if (skeletalMeshAnimation_.skeletalAnimation)
	{
		skeletalMeshAnimation_.elapsedTimeInSeconds += engine->GetDeltaTime();
		skeletalMeshAnimation_.animationTime = fmod(skeletalMeshAnimation_.skeletalAnimation->ticksPerSecond * skeletalMeshAnimation_.elapsedTimeInSeconds, skeletalMeshAnimation_.skeletalAnimation->duration);
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

void SkeletalMeshInstance::PlayAnimation(const std::string& animationName)
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
