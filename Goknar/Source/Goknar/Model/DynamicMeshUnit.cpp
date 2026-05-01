#include "pch.h"

#include "DynamicMeshUnit.h"

#include "Engine.h"
#include "Renderer/Renderer.h"

DynamicMeshUnit::DynamicMeshUnit() :
	MeshUnit(),
	rendererVertexOffset_(0)
{
}

DynamicMeshUnit::~DynamicMeshUnit()
{
}

void DynamicMeshUnit::PreInit()
{
}

void DynamicMeshUnit::Init()
{
}

void DynamicMeshUnit::PostInit()
{
}

void DynamicMeshUnit::UpdateVertexDataAt(int index, const VertexData& vertexData)
{
	engine->GetRenderer()->UpdateDynamicMeshVertex(this, index, vertexData);
}
