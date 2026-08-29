#include "pch.h"

#include "DynamicMeshLOD.h"

#include "Goknar/Engine.h"
#include "Goknar/Renderer/Renderer.h"

DynamicMeshLOD::DynamicMeshLOD() :
	MeshSection()
{
}

DynamicMeshLOD::DynamicMeshLOD(const MeshSection<DynamicMeshGeometry>& mesh) :
	MeshSection(mesh)
{
}

DynamicMeshLOD::~DynamicMeshLOD()
{
}

void DynamicMeshLOD::PreInit()
{
	MeshSection::PreInit();

	engine->AddDynamicMeshToRenderer(this);
}

void DynamicMeshLOD::Init()
{
	MeshSection::Init();
}

void DynamicMeshLOD::PostInit()
{
	MeshSection::PostInit();
}
