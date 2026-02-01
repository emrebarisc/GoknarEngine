#include "Bullet.h"

#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Physics/Components/SphereCollisionComponent.h"

Bullet::Bullet()
{
	SphereCollisionComponent* sphereCollisionComponent = AddSubComponent<SphereCollisionComponent>();
	sphereCollisionComponent->SetRadius(0.05f);

	AddSubComponent<StaticMeshComponent>();
}