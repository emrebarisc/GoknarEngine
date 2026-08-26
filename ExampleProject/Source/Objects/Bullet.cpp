#include "Bullet.h"

#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/MeshContainer.h"
#include "Goknar/Physics/Components/SphereCollisionComponent.h"

Bullet::Bullet()
{
	SphereCollisionComponent* sphereCollisionComponent = AddSubComponent<SphereCollisionComponent>();
	sphereCollisionComponent->SetRadius(0.05f);

	StaticMeshContainer* staticMesh = engine->GetResourceManager()->GetContent<StaticMeshContainer>("Meshes/SM_Bullet.fbx");

	StaticMeshComponent* staticMeshComponent = AddSubComponent<StaticMeshComponent>();
	staticMeshComponent->SetMesh(staticMesh);

	SetCollisionGroup(CollisionGroup::AllBlock);

	SetMass(2.f);
	SetCcdMotionThreshold(0.1f);
}
