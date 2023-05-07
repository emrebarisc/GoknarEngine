#include "PhysicsObjectManager.h"

#include "Goknar/Engine.h"
#include "Goknar/Physics/PhysicsWorld.h"
#include "Goknar/Physics/ForceGenerators/PhysicsObjectForceGenerator.h"

#include "Objects/Object.h"

PhysicsObjectManager::PhysicsObjectManager()
{
	SetIsTickable(true);
}

void PhysicsObjectManager::BeginGame()
{
	{
		Object* gravityObject = new Object();
		gravityObject->SetIsGravityEnabled(false);
		gravityObject->SetWorldPosition(Vector3{ 0.f, 16.f, 10.f });
		PhysicsObjectGravityForceGenerator* gravityForceGenerator = new PhysicsObjectGravityForceGenerator(DEFAULT_GRAVITATIONAL_FORCE);
		engine->GetPhysicsWorld()->GetPhysicsObjectForceRegistry()->Add(gravityObject, gravityForceGenerator);
	}

	{
		Object* dragObject = new Object();
		dragObject->SetIsGravityEnabled(true);
		dragObject->SetWorldPosition(Vector3{ 0.f, 20.f, 10.f });
		PhysicsObjectDragForceGenerator* dragForceGenerator = new PhysicsObjectDragForceGenerator(4.f, 8.f);
		engine->GetPhysicsWorld()->GetPhysicsObjectForceRegistry()->Add(dragObject, dragForceGenerator);
	}

	{
		Object* anchoredSpringObject = new Object();
		anchoredSpringObject->SetIsGravityEnabled(true);
		anchoredSpringObject->SetWorldPosition(Vector3{ 0.f, 24.f, 10.f });
		anchoredSpringObject->SetDamping(0.9f);

		anchoredSpringAnchor_ = Vector3(0.f, 24.f, 10.f);
		PhysicsObjectAnchoredSpringForceGenerator* anchoredSpringForceGenerator = new PhysicsObjectAnchoredSpringForceGenerator(&anchoredSpringAnchor_, 1.f, 2.f);
		engine->GetPhysicsWorld()->GetPhysicsObjectForceRegistry()->Add(anchoredSpringObject, anchoredSpringForceGenerator);
	}

	{
		Object* anchoredBungeeObject = new Object();
		anchoredBungeeObject->SetIsGravityEnabled(true);
		anchoredBungeeObject->SetWorldPosition(Vector3{ 0.f, 28.f, 10.f });
		anchoredBungeeObject->SetDamping(0.9f);

		anchoredBungeeAnchor_ = Vector3(0.f, 28.f, 8.f);
		PhysicsObjectAnchoredBungeeForceGenerator* anchoredBungeeForceGenerator = new PhysicsObjectAnchoredBungeeForceGenerator(&anchoredBungeeAnchor_, 1.f, 2.f);
		engine->GetPhysicsWorld()->GetPhysicsObjectForceRegistry()->Add(anchoredBungeeObject, anchoredBungeeForceGenerator);
	}

	{
		Object* springObject1 = new Object();
		springObject1->SetIsGravityEnabled(false);
		springObject1->SetWorldPosition(Vector3{ 0.f, 32.f, 10.f });

		Object* springObject2 = new Object();
		springObject2->SetWorldPosition(Vector3{ 0.f, 32.f, 0.f });

		PhysicsObjectSpringForceGenerator* springForceGenerator = new PhysicsObjectSpringForceGenerator(springObject1, 1.f, 2.f);
		engine->GetPhysicsWorld()->GetPhysicsObjectForceRegistry()->Add(springObject2, springForceGenerator);
	}

	{
		Object* bungeeObject1 = new Object();
		bungeeObject1->SetIsGravityEnabled(false);
		bungeeObject1->SetWorldPosition(Vector3{ 0.f, 36.f, 10.f });

		Object* bungeeObject2 = new Object();
		bungeeObject2->SetWorldPosition(Vector3{ 0.f, 36.f, 8.f });
		bungeeObject2->SetDamping(0.9f);

		PhysicsObjectBungeeForceGenerator* bungeeForceGenerator = new PhysicsObjectBungeeForceGenerator(bungeeObject1, 1.f, 5.f);
		engine->GetPhysicsWorld()->GetPhysicsObjectForceRegistry()->Add(bungeeObject2, bungeeForceGenerator);
	}

	{
		Object* bouyancyObject = new Object();
		//bouyancyObject->SetIsGravityEnabled(false);
		bouyancyObject->SetWorldPosition(Vector3{ 0.f, 40.f, 2.f });
		bouyancyObject->SetMass(2500.f);

		PhysicsObjectBuoyancyForceGenerator* buoyancyForceGenerator = new PhysicsObjectBuoyancyForceGenerator(0.5f, 100.f, 0.f, 500.f);
		engine->GetPhysicsWorld()->GetPhysicsObjectForceRegistry()->Add(bouyancyObject, buoyancyForceGenerator);
	}
}

void PhysicsObjectManager::Tick(float deltaTime)
{
	static float countdown = 1.f;

	if (countdown < 0.f)
	{
		Object* pistol = new Object();
		pistol->SetWorldPosition(Vector3::ZeroVector);
		pistol->SetIsGravityEnabled(false);
		pistol->SetMass(2.0f);
		pistol->SetVelocity(35.f, 0.f, 0.f);
		pistol->SetAcceleration(0.f, 0.f, -1.0f);
		pistol->SetDamping(0.99f);

		Object* artillery = new Object();
		artillery->SetWorldPosition(Vector3{ 0.f, 4.f, 0.f });
		artillery->SetIsGravityEnabled(false);
		artillery->SetMass(200.f);
		artillery->SetVelocity(40.f, 0.f, 30.f);
		artillery->SetAcceleration(0.f, 0.f, -20.f);
		artillery->SetDamping(0.99f);

		Object* fireball = new Object();
		fireball->SetWorldPosition(Vector3{ 0.f, 8.f, 0.f });
		fireball->SetIsGravityEnabled(false);
		fireball->SetMass(1.0f);
		fireball->SetVelocity(10.f, 0.f, 0.f);
		fireball->SetAcceleration(0.f, 0.f, 0.6f);
		fireball->SetDamping(0.9f);

		Object* laser = new Object();
		laser->SetWorldPosition(Vector3{ 0.f, 12.f, 0.f });
		laser->SetIsGravityEnabled(false);
		laser->SetMass(0.1f);
		laser->SetVelocity(100.f, 0.f, 0.f);
		laser->SetAcceleration(0.f, 0.f, 0.f);
		laser->SetDamping(0.99f);

		countdown = 1.f;
	}

	countdown -= deltaTime;
}
