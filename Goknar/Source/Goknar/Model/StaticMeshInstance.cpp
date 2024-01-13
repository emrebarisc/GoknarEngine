#include "pch.h"

#include "StaticMeshInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Renderer/Renderer.h"

StaticMeshInstance::StaticMeshInstance(RenderComponent* parentComponent) : 
	IMeshInstance(parentComponent)
{

}

void StaticMeshInstance::Render(RenderPassType renderPassType)
{
	PreRender(renderPassType);
	IMeshInstance::Render(renderPassType);
}

void StaticMeshInstance::AddMeshInstanceToRenderer()
{
	engine->GetRenderer()->AddStaticMeshInstance(this);
}

void StaticMeshInstance::RemoveMeshInstanceFromRenderer()
{
	engine->GetRenderer()->RemoveStaticMeshInstance(this);
}