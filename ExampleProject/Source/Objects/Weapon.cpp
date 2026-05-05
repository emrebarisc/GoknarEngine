#include "Weapon.h"

#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Managers/ResourceManager.h"

Weapon::Weapon() : ObjectBase()
{
	StaticMeshComponent* staticMeshComponent = AddSubComponent<StaticMeshComponent>();

	StaticMesh* staticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/Objects/SM_Weapon.fbx");

	staticMeshComponent->SetMesh(staticMesh);
}