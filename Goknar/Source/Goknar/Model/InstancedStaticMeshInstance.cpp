#include "pch.h"

#include "InstancedStaticMeshInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Renderer/Renderer.h"

InstancedStaticMeshInstance::InstancedStaticMeshInstance(RenderComponent* parentComponent) :
	IMeshInstance(parentComponent)
{
}

void InstancedStaticMeshInstance::Render(RenderPassType renderPassType, int subMeshIndex, int LODIndex)
{
	IMeshInstance::Render(renderPassType, subMeshIndex, LODIndex);
}

void InstancedStaticMeshInstance::AddMeshInstanceToRenderer()
{
	engine->GetRenderer()->AddInstancedStaticMeshInstance(this);
}

void InstancedStaticMeshInstance::RemoveMeshInstanceFromRenderer()
{
	engine->GetRenderer()->RemoveInstancedStaticMeshInstance(this);
}
