#include "pch.h"

#include "DynamicMeshInstance.h"

#include "Goknar/Application.h"
#include "Goknar/Model/DynamicMesh.h"
#include "Goknar/Engine.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Scene.h"

DynamicMeshInstance::DynamicMeshInstance(Component* parentComponent) : 
	MeshInstance(parentComponent)
{

}

void DynamicMeshInstance::UpdateVertexDataAt(int index, const VertexData& newVertexData)
{
	engine->GetRenderer()->UpdateDynamicMeshVertex(static_cast<DynamicMesh*>(mesh_), index, newVertexData);
}

void DynamicMeshInstance::AddMeshInstanceToRenderer()
{
	engine->GetRenderer()->AddDynamicMeshInstance(this);
}

void DynamicMeshInstance::RemoveMeshInstanceFromRenderer()
{
	engine->GetRenderer()->RemoveDynamicMeshInstance(this);
}

void DynamicMeshInstance::Destroy()
{
	RemoveMeshInstanceFromRenderer();
	delete this;
}

void DynamicMeshInstance::SetMesh(Mesh* mesh)
{
	mesh_ = mesh;
	AddMeshInstanceToRenderer();
}