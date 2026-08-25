#include "pch.h"

#include "DynamicMeshInstance.h"

#include "Goknar/Application.h"
#include "Goknar/Model/DynamicMesh.h"
#include "Goknar/Engine.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Scene.h"

DynamicMeshInstance::DynamicMeshInstance(RenderComponent* parentComponent) :
	IMeshInstance(parentComponent)
{

}

void DynamicMeshInstance::Render(RenderPassType renderPassType, int subMeshIndex, int LODIndex/* = 0*/)
{
	IMeshInstance::Render(renderPassType, subMeshIndex, LODIndex);
}

void DynamicMeshInstance::UpdateVertexDataAt(int meshIndex, int vertexIndex, const VertexData& newVertexData)
{
	engine->GetRenderer()->UpdateDynamicMeshVertex(mesh_->GetLOD(0)->GetSubMeshes()[meshIndex], vertexIndex, newVertexData);
}

void DynamicMeshInstance::AddMeshInstanceToRenderer()
{
	engine->GetRenderer()->AddDynamicMeshInstance(this);
}

void DynamicMeshInstance::RemoveMeshInstanceFromRenderer()
{
	engine->GetRenderer()->RemoveDynamicMeshInstance(this);
}