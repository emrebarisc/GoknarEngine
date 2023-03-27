#include "pch.h"

#include "MeshInstance.h"

#include "Goknar/Components/RenderComponent.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Model/MeshUnit.h"
#include "Goknar/Renderer/Renderer.h"

int MeshInstance::lastComponentId_ = 0;

void MeshInstance::PreRender()
{
	mesh_->GetMaterial()->Use();
}

void MeshInstance::Render()
{
	//Matrix relativeTransformationMatrix = relativeTransformationMatrix_;

	//// TODO: Optimize ////////
	//Component* parent = parentComponent_;
	//while (parent != nullptr)
	//{
	//	relativeTransformationMatrix *= parent->GetRelativeTransformationMatrix();
	//	parent = parent->GetParent();
	//}
	////////////////////////////
	mesh_->GetMaterial()->SetShaderVariables(worldTransformationMatrix_, relativeTransformationMatrix_);
}

void MeshInstance::SetMesh(MeshUnit* mesh)
{
	mesh_ = mesh;
	AddMeshInstanceToRenderer();
}

void MeshInstance::Destroy()
{
	RemoveMeshInstanceFromRenderer();
	delete this;
}

