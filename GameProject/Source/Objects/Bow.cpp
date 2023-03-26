#include "Bow.h"

#include "Goknar/Engine.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Components/StaticMeshComponent.h"

Bow::Bow() : StaticMeshObject()
{
	StaticMesh* bowStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Bow.fbx");
	staticMeshComponent->SetMesh(bowStaticMesh);
}