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
	{
		RigidBodyObject* gravityObject = new RigidBodyObject();
		gravityObject->SetIsGravityEnabled(false);
		gravityObject->SetWorldPosition(Vector3{ 0.f, 16.f, 10.f });
		Gravity* gravityForceGenerator = new Gravity(DEFAULT_GRAVITATIONAL_FORCE);
		engine->GetPhysicsWorld()->GetForceRegistry()->Add(gravityObject, gravityForceGenerator);
	}

	{
		RigidBodyObject* dragObject = new RigidBodyObject();
		dragObject->SetIsGravityEnabled(true);
		dragObject->SetWorldPosition(Vector3{ 0.f, 20.f, 10.f });
		DragForceGenerator* dragForceGenerator = new DragForceGenerator(4.f, 8.f);
		engine->GetPhysicsWorld()->GetForceRegistry()->Add(dragObject, dragForceGenerator);
	}

	{
		RigidBodyObject* anchoredSpringObject = new RigidBodyObject();
		anchoredSpringObject->SetIsGravityEnabled(true);
		anchoredSpringObject->SetWorldPosition(Vector3{ 0.f, 24.f, 10.f });
		anchoredSpringObject->SetDamping(0.9f, 0.9f);

		anchoredSpringAnchor_ = Vector3(0.f, 24.f, 10.f);
		AnchoredSpringForceGenerator* anchoredSpringForceGenerator = new AnchoredSpringForceGenerator(&anchoredSpringAnchor_, Vector3::ZeroVector, 1.f, 2.f);
		engine->GetPhysicsWorld()->GetForceRegistry()->Add(anchoredSpringObject, anchoredSpringForceGenerator);
	}

	{
		RigidBodyObject* anchoredSpringObject = new RigidBodyObject();
		anchoredSpringObject->SetIsGravityEnabled(true);
		anchoredSpringObject->SetWorldPosition(Vector3{ 0.f, 28.f, 10.f });
		anchoredSpringObject->SetDamping(0.9f, 0.9f);

		anchoredSpringWithRelativePositionAnchor_ = Vector3(0.f, 28.f, 10.f);
		AnchoredSpringForceGenerator* anchoredSpringForceGenerator = new AnchoredSpringForceGenerator(&anchoredSpringWithRelativePositionAnchor_, Vector3{ 1.f, 1.f, 1.f}, 1.f, 2.f);
		engine->GetPhysicsWorld()->GetForceRegistry()->Add(anchoredSpringObject, anchoredSpringForceGenerator);
	}

	{
		RigidBodyObject* anchoredBungeeObject = new RigidBodyObject();
		anchoredBungeeObject->SetIsGravityEnabled(true);
		anchoredBungeeObject->SetWorldPosition(Vector3{ 0.f, 32.f, 10.f });
		anchoredBungeeObject->SetDamping(0.9f, 0.9f);

		anchoredBungeeAnchor_ = Vector3(0.f, 32.f, 8.f);
		AnchoredBungeeForceGenerator* anchoredBungeeForceGenerator = new AnchoredBungeeForceGenerator(&anchoredBungeeAnchor_, Vector3{ 0.5f, 0.5f, 0.5f }, 1.f, 2.f);
		engine->GetPhysicsWorld()->GetForceRegistry()->Add(anchoredBungeeObject, anchoredBungeeForceGenerator);
	}

	{
		RigidBodyObject* springObject1 = new RigidBodyObject();
		springObject1->SetIsGravityEnabled(false);
		springObject1->SetWorldPosition(Vector3{ 0.f, 36.f, 10.f });

		RigidBodyObject* springObject2 = new RigidBodyObject();
		springObject2->SetWorldPosition(Vector3{ 0.f, 36.f, 0.f });

		Spring* springForceGenerator = new Spring(Vector3::ZeroVector, springObject1, Vector3::ZeroVector, 1.f, 2.f);
		engine->GetPhysicsWorld()->GetForceRegistry()->Add(springObject2, springForceGenerator);
	}

	{
		RigidBodyObject* bungeeObject1 = new RigidBodyObject();
		bungeeObject1->SetIsGravityEnabled(false);
		bungeeObject1->SetWorldPosition(Vector3{ 0.f, 40.f, 10.f });

		RigidBodyObject* bungeeObject2 = new RigidBodyObject();
		bungeeObject2->SetWorldPosition(Vector3{ 0.f, 40.f, 8.f });
		bungeeObject2->SetDamping(0.9f, 0.9f);

		BungeeForceGenerator* bungeeForceGenerator = new BungeeForceGenerator(bungeeObject1, 1.f, 5.f);
		engine->GetPhysicsWorld()->GetForceRegistry()->Add(bungeeObject2, bungeeForceGenerator);
	}

	{
		RigidBodyObject* bouyancyObject = new RigidBodyObject();
		bouyancyObject->SetWorldPosition(Vector3{ 0.f, 44.f, 2.f });
		bouyancyObject->SetMass(100.f);

		Buoyancy* buoyancyForceGenerator = new Buoyancy(0.5f, 100.f, 0.f, 1000.f);
		engine->GetPhysicsWorld()->GetForceRegistry()->Add(bouyancyObject, buoyancyForceGenerator);
	}

	{
		RigidBodyObject* aeroObject = new RigidBodyObject();
		aeroObject->SetIsGravityEnabled(false);
		aeroObject->SetWorldPosition(Vector3{ 0.f, 48.f, 0.f });
		aeroObject->SetMass(100.f);

		windSpeed_ = Vector3{ 10.f, 0.f, 0.f };

		Aero* aeroForceGenerator = new Aero(Quaternion::Identity.GetMatrix3x3(), Vector3{ 0.f, 48.f, 0.f }, &windSpeed_);
		engine->GetPhysicsWorld()->GetForceRegistry()->Add(aeroObject, aeroForceGenerator);
	}

	{
		RigidBodyObject* aeroControlObject = new RigidBodyObject();
		aeroControlObject->SetIsGravityEnabled(false);
		aeroControlObject->SetWorldPosition(Vector3{ 0.f, 52.f, 0.f });
		aeroControlObject->SetMass(100.f);

		windSpeed_ = Vector3{ 100.f, 0.f, 0.f };

		aeroControlForceGenerator = new AeroControl(Quaternion::Identity.GetMatrix3x3(), Quaternion(TWO_PI, 0.f, 0.f).GetMatrix3x3(), Quaternion(0.f, TWO_PI, 0.f).GetMatrix3x3(), Vector3{ 0.f, 52.f, 0.f }, &windSpeed_);
		engine->GetPhysicsWorld()->GetForceRegistry()->Add(aeroControlObject, aeroControlForceGenerator);
	}
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

	float oneTenThousandOfElapsedTime = engine->GetElapsedTime() * 0.0001f;
	aeroControlForceGenerator->SetControl(oneTenThousandOfElapsedTime - std::floor(oneTenThousandOfElapsedTime));

	countdown -= deltaTime;
}
