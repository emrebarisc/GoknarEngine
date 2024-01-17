#include "pch.h"

#include "StaticMesh.h"

#include "Goknar/Engine.h"

StaticMesh::StaticMesh() :
	MeshUnit()
{
	meshType_ = MeshType::Static;
}

StaticMesh::~StaticMesh()
{
}

void StaticMesh::PreInit()
{
	MeshUnit::PreInit();

	engine->AddStaticMeshToRenderer(this);
}

void StaticMesh::Init()
{
	MeshUnit::Init();
}

void StaticMesh::PostInit()
{
	MeshUnit::PostInit();
}
