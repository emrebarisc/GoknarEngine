#include "pch.h"

#include "DynamicMeshInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Renderer/Renderer.h"

DynamicMeshInstance::DynamicMeshInstance(Component* parentComponent) : MeshInstance(parentComponent)
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
