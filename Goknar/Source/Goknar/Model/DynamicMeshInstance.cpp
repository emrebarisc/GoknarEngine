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

void DynamicMeshInstance::Render(RenderPassType renderPassType)
{
	IMeshInstance::Render(renderPassType);
}

void DynamicMeshInstance::UpdateVertexDataAt(int meshIndex, int vertexIndex, const VertexData& newVertexData)
{
	engine->GetRenderer()->UpdateDynamicMeshVertex(mesh_->GetSubMeshes()[meshIndex], vertexIndex, newVertexData);
}

void DynamicMeshInstance::AddMeshInstanceToRenderer()
{
	engine->GetRenderer()->AddDynamicMeshInstance(this);
}

void DynamicMeshInstance::RemoveMeshInstanceFromRenderer()
{
	engine->GetRenderer()->RemoveDynamicMeshInstance(this);
}