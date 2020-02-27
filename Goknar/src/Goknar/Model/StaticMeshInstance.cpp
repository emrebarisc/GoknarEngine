#include "pch.h"

#include "StaticMeshInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Renderer/Renderer.h"

StaticMeshInstance::StaticMeshInstance(Component* parentComponent) : MeshInstance(parentComponent)
{

}

void StaticMeshInstance::AddMeshInstanceToRenderer()
{
	engine->GetRenderer()->AddStaticMeshInstance(this);
}

void StaticMeshInstance::RemoveMeshInstanceFromRenderer()
{
	engine->GetRenderer()->RemoveStaticMeshInstance(this);
}

void StaticMeshInstance::Destroy()
{
	RemoveMeshInstanceFromRenderer();
	delete this;
}
