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

<<<<<<< HEAD
void DynamicMeshInstance::Render(RenderPassType renderPassType, int subMeshIndex, int LODIndex/* = 0*/)
{
	IMeshInstance::Render(renderPassType, subMeshIndex, LODIndex);
=======
void DynamicMeshInstance::Render(int subMeshIndex, RenderPassType renderPassType)
{
	IMeshInstance::Render(subMeshIndex, renderPassType);
>>>>>>> master
}

void DynamicMeshInstance::UpdateVertexDataAt(int meshIndex, int vertexIndex, const VertexData& newVertexData)
{
<<<<<<< HEAD
	engine->GetRenderer()->UpdateDynamicMeshVertex(mesh_->GetLOD(0)->GetSubMeshes()[meshIndex], vertexIndex, newVertexData);
=======
	engine->GetRenderer()->UpdateDynamicMeshVertex(mesh_->GetSubMeshes()[meshIndex], vertexIndex, newVertexData);
>>>>>>> master
}

void DynamicMeshInstance::AddMeshInstanceToRenderer()
{
	engine->GetRenderer()->AddDynamicMeshInstance(this);
}

void DynamicMeshInstance::RemoveMeshInstanceFromRenderer()
{
	engine->GetRenderer()->RemoveDynamicMeshInstance(this);
}