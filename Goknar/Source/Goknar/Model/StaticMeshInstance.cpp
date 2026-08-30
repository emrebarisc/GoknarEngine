#include "pch.h"

#include "StaticMeshInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Renderer/Renderer.h"

StaticMeshInstance::StaticMeshInstance(RenderComponent* parentComponent) : 
	IMeshInstance(parentComponent)
{

}

<<<<<<< HEAD
void StaticMeshInstance::Render(RenderPassType renderPassType, int subMeshIndex, int LODIndex)
{
	IMeshInstance::Render(renderPassType, subMeshIndex, LODIndex);
=======
void StaticMeshInstance::Render(int subMeshIndex, RenderPassType renderPassType)
{
	IMeshInstance::Render(subMeshIndex, renderPassType);
>>>>>>> master
}

void StaticMeshInstance::AddMeshInstanceToRenderer()
{
	engine->GetRenderer()->AddStaticMeshInstance(this);
}

void StaticMeshInstance::RemoveMeshInstanceFromRenderer()
{
	engine->GetRenderer()->RemoveStaticMeshInstance(this);
}