#include "pch.h"

#include "DynamicMeshGeometry.h"

#include "Engine.h"
#include "Renderer/Renderer.h"

DynamicMeshGeometry::DynamicMeshGeometry() :
	MeshGeometry(),
	rendererVertexOffset_(0)
{
}

DynamicMeshGeometry::~DynamicMeshGeometry()
{
}

void DynamicMeshGeometry::PreInit()
{
}

void DynamicMeshGeometry::Init()
{
}

void DynamicMeshGeometry::PostInit()
{
}

void DynamicMeshGeometry::UpdateVertexDataAt(int index, const VertexData& vertexData)
{
	engine->GetRenderer()->UpdateDynamicMeshVertex(this, index, vertexData);
}
