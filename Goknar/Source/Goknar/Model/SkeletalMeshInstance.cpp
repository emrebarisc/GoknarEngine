#include "pch.h"

#include "SkeletalMeshInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Renderer/Renderer.h"

SkeletalMeshInstance::SkeletalMeshInstance(Component* parentComponent) :
	MeshInstance(parentComponent)
{
}

void SkeletalMeshInstance::Render()
{
	MeshInstance::Render();

	static bool trigerredAnimation = false;
	if (!trigerredAnimation)
	{
		trigerredAnimation = true;
		PlayAnimation("Armature|Armature|mixamo.com|Layer0");
		//PlayAnimation("Armature|Armature|Take 001|BaseLayer");
	}

	if (skeletalMeshAnimation_.skeletalAnimation)
	{
		skeletalMeshAnimation_.elapsedTimeInSeconds += engine->GetDeltaTime();
		skeletalMeshAnimation_.animationTime = fmod(skeletalMeshAnimation_.skeletalAnimation->ticksPerSecond * skeletalMeshAnimation_.elapsedTimeInSeconds, skeletalMeshAnimation_.skeletalAnimation->duration);
	}

	std::vector<Matrix> boneTransformations;
	/*TODO_Baris: TEMP*/dynamic_cast<SkeletalMesh*>(mesh_)/**/->GetBoneTransforms(boneTransformations, skeletalMeshAnimation_.skeletalAnimation, skeletalMeshAnimation_.animationTime);
	mesh_->GetMaterial()->GetShader()->SetMatrixVector("bones", boneTransformations);
}

void SkeletalMeshInstance::PlayAnimation(const std::string& animationName)
{
	if (!skeletalMeshAnimation_.skeletalAnimation || 
		skeletalMeshAnimation_.skeletalAnimation && skeletalMeshAnimation_.skeletalAnimation->name != animationName)
	{
		SkeletalMesh* skeletalMesh = /*TODO_Baris: TEMP*/dynamic_cast<SkeletalMesh*>(GetMesh())/**/;
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