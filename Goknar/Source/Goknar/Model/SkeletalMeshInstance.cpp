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

void SkeletalMeshInstance::Render() const
{
	MeshInstance::Render();

	// TODO_Baris: TEMP
	std::vector<Matrix> boneTransformations;
	dynamic_cast<SkeletalMesh*>(mesh_)->GetBoneTransforms(boneTransformations);
	mesh_->GetMaterial()->GetShader()->SetMatrixVector("bones", boneTransformations);
}

void SkeletalMeshInstance::AddMeshInstanceToRenderer()
{
	engine->GetRenderer()->AddSkeletalMeshInstance(this);
}

void SkeletalMeshInstance::RemoveMeshInstanceFromRenderer()
{
	engine->GetRenderer()->RemoveSkeletalMeshInstance(this);
}