#include "Bullet.h"

#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Physics/Components/SphereCollisionComponent.h"

Bullet::Bullet()
{
	SphereCollisionComponent* sphereCollisionComponent = AddSubComponent<SphereCollisionComponent>();
	sphereCollisionComponent->SetRadius(0.05f);

	StaticMesh* staticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Bullet.fbx");

	StaticMeshComponent* staticMeshComponent = AddSubComponent<StaticMeshComponent>();
	staticMeshComponent->SetMesh(staticMesh);

	SetCollisionGroup(CollisionGroup::AllBlock);

	SetMass(2.f);
	SetCcdMotionThreshold(0.1f);
}