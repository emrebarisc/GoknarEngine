#include "pch.h"

#include "MeshInstance.h"

#include "Goknar/Components/Component.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Model/Mesh.h"
#include "Goknar/Renderer/Renderer.h"

int MeshInstance::lastComponentId_ = 0;

void MeshInstance::Render(Material* renderMaterial) const
{
	if (renderMaterial == nullptr)
	{
		mesh_->GetMaterial()->Render(worldTransformationMatrix_, relativeTransformationMatrix_);
	}
	else
	{
		renderMaterial->Render(worldTransformationMatrix_, relativeTransformationMatrix_);
	}
}