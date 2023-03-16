#include "pch.h"

#include "MeshInstance.h"

#include "Goknar/Components/Component.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Model/MeshUnit.h"
#include "Goknar/Renderer/Renderer.h"

int MeshInstance::lastComponentId_ = 0;

void MeshInstance::Render()
{
	mesh_->GetMaterial()->Render(worldTransformationMatrix_, relativeTransformationMatrix_);
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

