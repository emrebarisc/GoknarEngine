#include "pch.h"

#include "MeshInstance.h"

#include "Goknar/Components/Component.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Model/Mesh.h"
#include "Goknar/Renderer/Renderer.h"

int MeshInstance::lastComponentId_ = 0;

void MeshInstance::RemoveFromRenderer()
{
	engine->GetRenderer()->RemoveMeshInstance(this);
}

void MeshInstance::SetMesh(Mesh* mesh)
{
	mesh_ = mesh;
	engine->GetRenderer()->AddMeshInstance(this);
}

void MeshInstance::Render() const
{
	mesh_->GetMaterial()->Render(worldTransformationMatrix_ * relativeTransformationMatrix_);
}