#include "pch.h"

#include "StaticMesh.h"

StaticMesh::StaticMesh() :
	Mesh()
{
	meshType_ = MeshType::Static;
}

StaticMesh::~StaticMesh()
{
}

void StaticMesh::Init()
{
	Mesh::Init();
}
