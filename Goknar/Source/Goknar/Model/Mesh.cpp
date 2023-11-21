#include "pch.h"

#include "Mesh.h"

#include "Material.h"
#include "MeshUnit.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
	std::vector<MeshUnit*>::iterator subMeshIterator = subMeshes_.begin();
	for (; subMeshIterator != subMeshes_.end(); ++subMeshIterator)
	{
		delete *subMeshIterator;
	}
	subMeshes_.clear();
}

void Mesh::AddMesh(MeshUnit* meshUnit)
{
	subMeshes_.push_back(meshUnit);
}

void Mesh::Init()
{
}

void Mesh::Render(RenderPassType renderPassType, const Matrix& worldAndRelativeTransformationMatrix)
{
	std::vector<MeshUnit*>::iterator subMeshIterator = subMeshes_.begin();
	for (; subMeshIterator != subMeshes_.end(); ++subMeshIterator)
	{
		Material* meshUnitMaterial = (*subMeshIterator)->GetMaterial();
		if (meshUnitMaterial)
		{
			meshUnitMaterial->Render(renderPassType, worldAndRelativeTransformationMatrix);
		}
	}
}
