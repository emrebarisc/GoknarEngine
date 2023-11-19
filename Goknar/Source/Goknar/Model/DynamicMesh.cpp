#include "pch.h"

#include "DynamicMesh.h"

#include "Goknar/Engine.h"
#include "Goknar/Renderer/Renderer.h"

DynamicMesh::DynamicMesh() :
	MeshUnit(),
	rendererVertexOffset_(0)
{
	meshType_ = MeshType::Dynamic;
}

DynamicMesh::DynamicMesh(const MeshUnit& mesh) :
	MeshUnit(mesh)
{
}

DynamicMesh::~DynamicMesh()
{
}

void DynamicMesh::PreInit()
{
	MeshUnit::PreInit();

	engine->AddDynamicMeshToRenderer(this);
}

void DynamicMesh::Init()
{
	MeshUnit::Init();
}

void DynamicMesh::PostInit()
{
	MeshUnit::PostInit();
}

void DynamicMesh::UpdateVertexDataAt(int index, const VertexData& vertexData)
{
	engine->GetRenderer()->UpdateDynamicMeshVertex(this, index, vertexData);
}
