#include "Weapon.h"

#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/MeshContainer.h"

Weapon::Weapon() : ObjectBase()
{
	StaticMeshComponent* staticMeshComponent = AddSubComponent<StaticMeshComponent>();

	StaticMeshContainer* staticMesh = engine->GetResourceManager()->GetContent<StaticMeshContainer>("Meshes/Objects/SM_Weapon.fbx");

	staticMeshComponent->SetMesh(staticMesh);
}
