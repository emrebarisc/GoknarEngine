#include "RigidBodyManager.h"

#include "Goknar/Engine.h"
#include "Goknar/Physics/PhysicsWorld.h"
#include "Goknar/Physics/ForceGenerators/ForceGenerator.h"

#include "Objects/RigidBodyObject.h"

RigidBodyManager::RigidBodyManager()
{
	SetIsTickable(true);
}

void RigidBodyManager::BeginGame()
{
	// {
	// 	RigidBodyObject* gravityRigidBodyObject = new RigidBodyObject();
	// 	gravityRigidBodyObject->SetIsGravityEnabled(false);
	// 	gravityRigidBodyObject->SetWorldPosition(Vector3{ 0.f, 16.f, 10.f });
	// 	RigidBodyGravityForceGenerator* gravityForceGenerator = new RigidBodyGravityForceGenerator(DEFAULT_GRAVITATIONAL_FORCE);
	// 	engine->GetPhysicsWorld()->GetRigidBodyForceRegistry()->Add(gravityRigidBodyObject, gravityForceGenerator);
	// }

	// {
	// 	RigidBodyObject* dragRigidBodyObject = new RigidBodyObject();
	// 	dragRigidBodyObject->SetIsGravityEnabled(true);
	// 	dragRigidBodyObject->SetWorldPosition(Vector3{ 0.f, 20.f, 10.f });
	// 	RigidBodyDragForceGenerator* dragForceGenerator = new RigidBodyDragForceGenerator(4.f, 8.f);
	// 	engine->GetPhysicsWorld()->GetRigidBodyForceRegistry()->Add(dragRigidBodyObject, dragForceGenerator);
	// }

	// {
	// 	RigidBodyObject* anchoredSpringRigidBodyObject = new RigidBodyObject();
	// 	anchoredSpringRigidBodyObject->SetIsGravityEnabled(true);
	// 	anchoredSpringRigidBodyObject->SetWorldPosition(Vector3{ 0.f, 24.f, 10.f });
	// 	anchoredSpringRigidBodyObject->SetDamping(0.9f);

	// 	anchoredSpringAnchor_ = Vector3(0.f, 24.f, 10.f);
	// 	RigidBodyAnchoredSpringForceGenerator* anchoredSpringForceGenerator = new RigidBodyAnchoredSpringForceGenerator(&anchoredSpringAnchor_, 1.f, 2.f);
	// 	engine->GetPhysicsWorld()->GetRigidBodyForceRegistry()->Add(anchoredSpringRigidBodyObject, anchoredSpringForceGenerator);
	// }

	// {
	// 	RigidBodyObject* anchoredBungeeRigidBodyObject = new RigidBodyObject();
	// 	anchoredBungeeRigidBodyObject->SetIsGravityEnabled(true);
	// 	anchoredBungeeRigidBodyObject->SetWorldPosition(Vector3{ 0.f, 28.f, 10.f });
	// 	anchoredBungeeRigidBodyObject->SetDamping(0.9f);

	// 	anchoredBungeeAnchor_ = Vector3(0.f, 28.f, 8.f);
	// 	RigidBodyAnchoredBungeeForceGenerator* anchoredBungeeForceGenerator = new RigidBodyAnchoredBungeeForceGenerator(&anchoredBungeeAnchor_, 1.f, 2.f);
	// 	engine->GetPhysicsWorld()->GetRigidBodyForceRegistry()->Add(anchoredBungeeRigidBodyObject, anchoredBungeeForceGenerator);
	// }

	// {
	// 	RigidBodyObject* springRigidBodyObject1 = new RigidBodyObject();
	// 	springRigidBodyObject1->SetIsGravityEnabled(false);
	// 	springRigidBodyObject1->SetWorldPosition(Vector3{ 0.f, 32.f, 10.f });

	// 	RigidBodyObject* springRigidBodyObject2 = new RigidBodyObject();
	// 	springRigidBodyObject2->SetWorldPosition(Vector3{ 0.f, 32.f, 0.f });

	// 	RigidBodySpringForceGenerator* springForceGenerator = new RigidBodySpringForceGenerator(springRigidBodyObject1, 1.f, 2.f);
	// 	engine->GetPhysicsWorld()->GetRigidBodyForceRegistry()->Add(springRigidBodyObject2, springForceGenerator);
	// }

	// {
	// 	RigidBodyObject* bungeeRigidBodyObject1 = new RigidBodyObject();
	// 	bungeeRigidBodyObject1->SetIsGravityEnabled(false);
	// 	bungeeRigidBodyObject1->SetWorldPosition(Vector3{ 0.f, 36.f, 10.f });

	// 	RigidBodyObject* bungeeRigidBodyObject2 = new RigidBodyObject();
	// 	bungeeRigidBodyObject2->SetWorldPosition(Vector3{ 0.f, 36.f, 8.f });
	// 	bungeeRigidBodyObject2->SetDamping(0.9f);

	// 	RigidBodyBungeeForceGenerator* bungeeForceGenerator = new RigidBodyBungeeForceGenerator(bungeeRigidBodyObject1, 1.f, 5.f);
	// 	engine->GetPhysicsWorld()->GetRigidBodyForceRegistry()->Add(bungeeRigidBodyObject2, bungeeForceGenerator);
	// }

	// {
	// 	RigidBodyObject* bouyancyRigidBodyObject = new RigidBodyObject();
	// 	//bouyancyRigidBodyObject->SetIsGravityEnabled(false);
	// 	bouyancyRigidBodyObject->SetWorldPosition(Vector3{ 0.f, 40.f, 2.f });
	// 	bouyancyRigidBodyObject->SetMass(2500.f);

	// 	RigidBodyBuoyancyForceGenerator* buoyancyForceGenerator = new RigidBodyBuoyancyForceGenerator(0.5f, 100.f, 0.f, 500.f);
	// 	engine->GetPhysicsWorld()->GetRigidBodyForceRegistry()->Add(bouyancyRigidBodyObject, buoyancyForceGenerator);
	// }
}

void RigidBodyManager::Tick(float deltaTime)
{
	static float countdown = 1.f;

	if (countdown < 0.f)
	{
		RigidBodyObject* pistol = new RigidBodyObject();
		pistol->SetWorldPosition(Vector3::ZeroVector);
		pistol->SetIsGravityEnabled(false);
		pistol->SetMass(2.0f);
		pistol->SetVelocity(35.f, 0.f, 0.f);
		pistol->SetAcceleration(0.f, 0.f, -1.0f);
		pistol->SetDamping(0.99f, 0.99f);

		RigidBodyObject* artillery = new RigidBodyObject();
		artillery->SetWorldPosition(Vector3{ 0.f, 4.f, 0.f });
		artillery->SetIsGravityEnabled(false);
		artillery->SetMass(200.f);
		artillery->SetVelocity(40.f, 0.f, 30.f);
		artillery->SetAcceleration(0.f, 0.f, -20.f);
		artillery->SetDamping(0.99f, 0.99f);

		RigidBodyObject* fireball = new RigidBodyObject();
		fireball->SetWorldPosition(Vector3{ 0.f, 8.f, 0.f });
		fireball->SetIsGravityEnabled(false);
		fireball->SetMass(1.0f);
		fireball->SetVelocity(10.f, 0.f, 0.f);
		fireball->SetAcceleration(0.f, 0.f, 0.6f);
		fireball->SetDamping(0.9f, 0.9f);

		RigidBodyObject* laser = new RigidBodyObject();
		laser->SetWorldPosition(Vector3{ 0.f, 12.f, 0.f });
		laser->SetIsGravityEnabled(false);
		laser->SetMass(0.1f);
		laser->SetVelocity(100.f, 0.f, 0.f);
		laser->SetAcceleration(0.f, 0.f, 0.f);
		laser->SetDamping(0.99f, 0.99f);

		RigidBodyObject* box = new RigidBodyObject();
		box->SetWorldPosition(Vector3{ 0.f, -4.f, 0.f });
		box->SetIsGravityEnabled(true);
		box->SetMass(1.f);
		box->SetDamping(0.99f, 0.99f);
		box->AddForceAtPoint(Vector3{ 4000.f, 0.f, 2000.f }, Vector3{ -0.5f, -4.f, -0.5f });
		box->AddTorque(Vector3{ 0.f, 0.f, 40000.f });

		countdown = 1.f;
	}

	countdown -= deltaTime;
}
