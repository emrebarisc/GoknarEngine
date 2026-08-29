#include "pch.h"

#include "StaticMeshInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Renderer/Renderer.h"

StaticMeshInstance::StaticMeshInstance(RenderComponent* parentComponent) : 
	IMeshInstance(parentComponent)
{

}

void StaticMeshInstance::Render(int subMeshIndex, RenderPassType renderPassType)
{
	IMeshInstance::Render(subMeshIndex, renderPassType);
}

void StaticMeshInstance::AddMeshInstanceToRenderer()
{
	engine->GetRenderer()->AddStaticMeshInstance(this);
}

void StaticMeshInstance::RemoveMeshInstanceFromRenderer()
{
	engine->GetRenderer()->RemoveStaticMeshInstance(this);
}