#include "pch.h"

#include "StaticMesh.h"

#include "Goknar/Engine.h"

StaticMesh::StaticMesh() :
	Mesh()
{
}

StaticMesh::~StaticMesh()
{
}

void StaticMesh::PreInit()
{
	Mesh::PreInit();

	engine->AddStaticMeshToRenderer(this);
}

void StaticMesh::Init()
{
	Mesh::Init();
}

void StaticMesh::PostInit()
{
	Mesh::PostInit();
}
