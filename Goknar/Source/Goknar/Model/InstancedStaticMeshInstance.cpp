#include "pch.h"

#include "InstancedStaticMeshInstance.h"

#include "Goknar/Engine.h"
#include "Goknar/Renderer/Renderer.h"

InstancedStaticMeshInstance::InstancedStaticMeshInstance(RenderComponent* parentComponent) :
	IMeshInstance(parentComponent)
{
}

void InstancedStaticMeshInstance::Render(int subMeshIndex, RenderPassType renderPassType)
{
	IMeshInstance::Render(subMeshIndex, renderPassType);
}

void InstancedStaticMeshInstance::AddMeshInstanceToRenderer()
{
	engine->GetRenderer()->AddInstancedStaticMeshInstance(this);
}

void InstancedStaticMeshInstance::RemoveMeshInstanceFromRenderer()
{
	engine->GetRenderer()->RemoveInstancedStaticMeshInstance(this);
}
