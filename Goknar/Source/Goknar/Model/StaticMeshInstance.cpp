#include "pch.h"

#include "StaticMeshInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Renderer/Renderer.h"

StaticMeshInstance::StaticMeshInstance(RenderComponent* parentComponent) :
	IMeshInstance(parentComponent)
{

}

void StaticMeshInstance::Render(RenderPassType renderPassType, int subMeshIndex, int LODIndex)
{
	IMeshInstance::Render(renderPassType, subMeshIndex, LODIndex);
}

void StaticMeshInstance::AddMeshInstanceToRenderer()
{
	engine->GetRenderer()->AddStaticMeshInstance(this);
}

void StaticMeshInstance::RemoveMeshInstanceFromRenderer()
{
	engine->GetRenderer()->RemoveStaticMeshInstance(this);
}