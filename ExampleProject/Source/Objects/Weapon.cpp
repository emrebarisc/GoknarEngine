#include "Weapon.h"

#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/Mesh.h"

Weapon::Weapon() : ObjectBase()
{
	StaticMeshComponent* staticMeshComponent = AddSubComponent<StaticMeshComponent>();

	StaticMesh* staticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/Objects/SM_Weapon.fbx");

	staticMeshComponent->SetMesh(staticMesh);
}
