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
	DynamicMesh* dynamicMesh = new DynamicMesh(*mesh);
	engine->GetApplication()->GetMainScene()->AddDynamicMesh(dynamicMesh);
	mesh_ = dynamicMesh;
	AddMeshInstanceToRenderer();
}