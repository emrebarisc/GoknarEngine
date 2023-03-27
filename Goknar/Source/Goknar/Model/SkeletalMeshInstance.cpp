#include "pch.h"

#include "SkeletalMeshInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/ShaderTypes.h"

SkeletalMeshInstance::SkeletalMeshInstance(RenderComponent* parentComponent) :
	MeshInstance(parentComponent)
{
}

void SkeletalMeshInstance::Render()
{
	PreRender();

	if (skeletalMeshAnimation_.skeletalAnimation)
	{
		skeletalMeshAnimation_.elapsedTimeInSeconds += engine->GetDeltaTime();
		skeletalMeshAnimation_.animationTime = fmod(skeletalMeshAnimation_.skeletalAnimation->ticksPerSecond * skeletalMeshAnimation_.elapsedTimeInSeconds, skeletalMeshAnimation_.skeletalAnimation->duration);
	}

	std::vector<Matrix> boneTransformations;
	mesh_->GetBoneTransforms(boneTransformations, skeletalMeshAnimation_.skeletalAnimation, skeletalMeshAnimation_.animationTime);
	mesh_->GetMaterial()->GetShader()->SetMatrixVector(SHADER_VARIABLE_NAMES::SKELETAL_MESH::BONES, boneTransformations);

	MeshInstance::Render();
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