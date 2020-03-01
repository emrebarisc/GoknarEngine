#include "pch.h"

#include "DynamicMesh.h"

#include "Goknar/Engine.h"

DynamicMesh::DynamicMesh() :
	Mesh()
{
	meshType_ = MeshType::Dynamic;
}

DynamicMesh::DynamicMesh(const Mesh& mesh) :
	Mesh(mesh)
{
}

DynamicMesh::~DynamicMesh()
{
}

void DynamicMesh::Init()
{
	Mesh::Init();
}

void DynamicMesh::UpdateVertexDataAt(int index, const VertexData& vertexData)
{
	//engine->GetRenderer()->
}
