#include "PhysicsObjectManager.h"

#include "Objects/Object.h"

PhysicsObjectManager::PhysicsObjectManager()
{
	SetIsTickable(true);
}

void PhysicsObjectManager::BeginGame()
{

}

void PhysicsObjectManager::Tick(float deltaTime)
{
	static float countdown = 1.f;

	if (countdown < 0.f)
	{
		Object* pistol = new Object();
		pistol->SetWorldPosition(Vector3::ZeroVector);
		pistol->SetMass(2.0f);
		pistol->SetVelocity(35.f, 0.f, 0.f);
		pistol->SetAcceleration(0.f, 0.f, -1.0f);
		pistol->SetDamping(0.99f);

		Object* artillery = new Object();
		artillery->SetWorldPosition(Vector3{ 0.f, 4.f, 0.f });
		artillery->SetMass(200.f);
		artillery->SetVelocity(40.f, 0.f, 30.f);
		artillery->SetAcceleration(0.f, 0.f, -20.f);
		artillery->SetDamping(0.99f);

		Object* fireball = new Object();
		fireball->SetWorldPosition(Vector3{ 0.f, 8.f, 0.f });
		fireball->SetMass(1.0f);
		fireball->SetVelocity(10.f, 0.f, 0.f);
		fireball->SetAcceleration(0.f, 0.f, 0.6f);
		fireball->SetDamping(0.9f);

		Object* laser = new Object();
		laser->SetWorldPosition(Vector3{ 0.f, 12.f, 0.f });
		laser->SetMass(0.1f);
		laser->SetVelocity(100.f, 0.f, 0.f);
		laser->SetAcceleration(0.f, 0.f, 0.f);
		laser->SetDamping(0.99f);

		countdown = 1.f;
	}

	countdown -= deltaTime;
}
