#include "pch.h"

#include "StaticMeshLOD.h"

#include "Goknar/Engine.h"

StaticMeshLOD::StaticMeshLOD() :
	MeshSection()
{
}

StaticMeshLOD::~StaticMeshLOD()
{
}

void StaticMeshLOD::PreInit()
{
	MeshSection::PreInit();

	engine->AddStaticMeshToRenderer(this);
}

void StaticMeshLOD::Init()
{
	MeshSection::Init();
}

void StaticMeshLOD::PostInit()
{
	MeshSection::PostInit();
}
