#include "pch.h"

#include "DynamicMesh.h"

#include "Goknar/Engine.h"
#include "Goknar/Renderer/Renderer.h"

DynamicMesh::DynamicMesh() :
	Mesh()
{
}

DynamicMesh::DynamicMesh(const Mesh& mesh) :
	Mesh(mesh)
{
}

DynamicMesh::~DynamicMesh()
{
}

void DynamicMesh::PreInit()
{
	Mesh::PreInit();

	engine->AddDynamicMeshToRenderer(this);
}

void DynamicMesh::Init()
{
	Mesh::Init();
}

void DynamicMesh::PostInit()
{
	Mesh::PostInit();
}