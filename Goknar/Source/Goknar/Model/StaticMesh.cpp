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

void StaticMesh::Init()
{
	MeshUnit::Init();

	engine->AddStaticMeshToRenderer(this);
}
