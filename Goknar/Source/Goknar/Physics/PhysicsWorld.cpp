#include "pch.h"

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"

#include "Engine.h"
#include "Log.h"
#include "PhysicsDebugger.h"
#include "PhysicsUtils.h"
#include "PhysicsWorld.h"
#include "RigidBody.h"
#include "Character.h"
#include "Components/PhysicsMovementComponent.h"
#include "Components/CollisionComponent.h"
#include "Components/OverlappingTypes.h"

PhysicsWorld::PhysicsWorld()
{
	physicsDebugger_ = new PhysicsDebugger();
}

PhysicsWorld::~PhysicsWorld()
{
	delete dynamicsWorld_;
	dynamicsWorld_ = nullptr;

	delete solver_;
	solver_ = nullptr;

	delete broadphase_;
	broadphase_ = nullptr;

	delete ghostPairCallback_;
	ghostPairCallback_ = nullptr;

	delete dispatcher_;
	dispatcher_ = nullptr;

	delete collisionConfiguration_;
	collisionConfiguration_ = nullptr;

	delete overlappingCollisionPairCallback_;
	overlappingCollisionPairCallback_ = nullptr;

	delete physicsDebugger_;
	physicsDebugger_ = nullptr;
}

bool OverlappingDestroyedCallback(void* userPersistentData)
{
	return true;
}

bool OverlappingProcessedCallback(btManifoldPoint& manifoldPoint, void* body0, void* body1)
{
	engine->GetPhysicsWorld()->OnOverlappingCollisionContinue(manifoldPoint, static_cast<const btCollisionObject*>(body0), static_cast<const btCollisionObject*>(body1));
	return true;
}

void OverlappingStartedCallback(btPersistentManifold* const& manifold)
{
	engine->GetPhysicsWorld()->OnOverlappingCollisionBegin(manifold);
}

void OverlappingEndedCallback(btPersistentManifold* const& manifold)
{
	engine->GetPhysicsWorld()->OnOverlappingCollisionEnd(manifold);
}

void PhysicsWorld::PreInit()
{
	///collision configuration contains default setup for memory, collision setup
	collisionConfiguration_ = new btDefaultCollisionConfiguration();
	//m_collisionConfiguration->setConvexConvexMultipointIterations();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	dispatcher_ = new btCollisionDispatcher(collisionConfiguration_);
	//dispatcher_->setNearCallback(&NearCallback);

	ghostPairCallback_ = new btGhostPairCallback();

	broadphase_ = new btDbvtBroadphase();
	broadphase_->getOverlappingPairCache()->setInternalGhostPairCallback(ghostPairCallback_);

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	solver_ = solver;

	dynamicsWorld_ = new btDiscreteDynamicsWorld(dispatcher_, broadphase_, solver_, collisionConfiguration_);
	dynamicsWorld_->setGravity(PhysicsUtils::FromVector3ToBtVector3(gravity_));
	dynamicsWorld_->setDebugDrawer(physicsDebugger_);

	//dynamicsWorld_->getDispatchInfo().m_allowedCcdPenetration = 0.04f;

	gContactDestroyedCallback = OverlappingDestroyedCallback;
	gContactProcessedCallback = OverlappingProcessedCallback;
	gContactStartedCallback = OverlappingStartedCallback;
	gContactEndedCallback = OverlappingEndedCallback;
}

void PhysicsWorld::Init()
{
}

void PhysicsWorld::PostInit()
{
}

void PhysicsWorld::PhysicsTick(float deltaTime)
{
	dynamicsWorld_->stepSimulation(deltaTime);

	for (PhysicsObject* physicsObject : physicsObjects_)
	{
		if (physicsObject->GetIsActive() && physicsObject->GetPhysicsTickEnabled())
		{
			physicsObject->PhysicsTick(deltaTime);
		}
	}

	for(PhysicsMovementComponent* physicsMovementComponent : physicsMovementComponents_)
	{
		physicsMovementComponent->UpdateOwnerTransformation();
	}
}

void PhysicsWorld::OnOverlappingCollisionBegin(btPersistentManifold* const& monifoldPointPtr)
{
	const btCollisionObject* ghostObject1 = monifoldPointPtr->getBody0();
	const btCollisionObject* ghostObject2 = monifoldPointPtr->getBody1();

	PhysicsObject* collisionObject1 = (PhysicsObject*)ghostObject1->getUserPointer();
	PhysicsObject* collisionObject2 = (PhysicsObject*)ghostObject2->getUserPointer();

	GOKNAR_CORE_ASSERT(collisionObject1 && collisionObject2, "Neither of the colliding bodies can be nullptr!");
	
	CollisionComponent* collisionComponent1 = collisionObject1->GetCollisionComponent();
	CollisionComponent* collisionComponent2 = collisionObject2->GetCollisionComponent();

	GOKNAR_CORE_ASSERT(collisionComponent1 && collisionComponent2);

	Vector3 worldPositionOnA = PhysicsUtils::FromBtVector3ToVector3(monifoldPointPtr->getContactPoint(0).getPositionWorldOnA());
	Vector3 hitNormal = PhysicsUtils::FromBtVector3ToVector3(monifoldPointPtr->getContactPoint(0).m_normalWorldOnB);

	Vector3 worldPositionOnB = PhysicsUtils::FromBtVector3ToVector3(monifoldPointPtr->getContactPoint(0).getPositionWorldOnB());

	collisionComponent1->OverlapBegin(collisionObject2, collisionComponent2, worldPositionOnA, hitNormal);
	collisionComponent2->OverlapBegin(collisionObject1, collisionComponent1, worldPositionOnB, hitNormal);
}

void PhysicsWorld::OnOverlappingCollisionContinue(btManifoldPoint& monifoldPoint, const btCollisionObject* ghostObject1, const btCollisionObject* ghostObject2)
{
	PhysicsObject* collisionObject1 = (PhysicsObject*)ghostObject1->getUserPointer();
	PhysicsObject* collisionObject2 = (PhysicsObject*)ghostObject2->getUserPointer();

	GOKNAR_CORE_ASSERT(collisionObject1 && collisionObject2, "Neither of the colliding bodies can be nullptr!");

	CollisionComponent* collisionComponent1 = collisionObject1->GetCollisionComponent();
	CollisionComponent* collisionComponent2 = collisionObject2->GetCollisionComponent();

	GOKNAR_CORE_ASSERT(collisionComponent1 && collisionComponent2);

	Vector3 worldPositionOnA = PhysicsUtils::FromBtVector3ToVector3(monifoldPoint.getPositionWorldOnA());
	Vector3 hitNormal = PhysicsUtils::FromBtVector3ToVector3(monifoldPoint.m_normalWorldOnB);

	Vector3 worldPositionOnB = PhysicsUtils::FromBtVector3ToVector3(monifoldPoint.getPositionWorldOnB());

	collisionComponent1->OverlapContinue(collisionObject2, collisionComponent2, worldPositionOnA, hitNormal);
	collisionComponent2->OverlapContinue(collisionObject1, collisionComponent1, worldPositionOnB, hitNormal);
}

void PhysicsWorld::OnOverlappingCollisionEnd(btPersistentManifold* const& manifold)
{
	PhysicsObject* collisionObject1 = (PhysicsObject*)manifold->getBody0()->getUserPointer();
	PhysicsObject* collisionObject2 = (PhysicsObject*)manifold->getBody1()->getUserPointer();

	GOKNAR_CORE_ASSERT(collisionObject1 && collisionObject2, "Neither of the colliding bodies can be nullptr!");

	CollisionComponent* collisionComponent1 = collisionObject1->GetCollisionComponent();
	CollisionComponent* collisionComponent2 = collisionObject2->GetCollisionComponent();

	GOKNAR_CORE_ASSERT(collisionComponent1 && collisionComponent2);

	collisionComponent1->OverlapEnd(collisionObject2, collisionComponent2);
	collisionComponent2->OverlapEnd(collisionObject1, collisionComponent1);
};

void PhysicsWorld::AddRigidBody(RigidBody* rigidBody)
{
	btRigidBody* bulletRigidBody = rigidBody->GetBulletRigidBody();

	dynamicsWorld_->addRigidBody(bulletRigidBody, (int)rigidBody->GetCollisionGroup(), (int)rigidBody->GetCollisionMask());
	physicsObjects_.push_back(rigidBody);
}

void PhysicsWorld::RemoveRigidBody(RigidBody* rigidBody)
{
	btRigidBody* bulletRigidBody = rigidBody->GetBulletRigidBody();

	if (!bulletRigidBody)
	{
		return;
	}

	decltype(physicsObjects_.begin()) physicsObjectIterator = physicsObjects_.begin();
	while(physicsObjectIterator != physicsObjects_.end())
	{
		if(*physicsObjectIterator == rigidBody)
		{
			physicsObjects_.erase(physicsObjectIterator);
			break;
		}

		++physicsObjectIterator;
	}

	dynamicsWorld_->removeRigidBody(bulletRigidBody);
}

void PhysicsWorld::AddPhysicsObject(PhysicsObject* physicsObject)
{
	dynamicsWorld_->addCollisionObject(physicsObject->GetBulletCollisionObject(), (int)physicsObject->GetCollisionGroup(), (int)physicsObject->GetCollisionMask());
	physicsObjects_.push_back(physicsObject);
}

void PhysicsWorld::RemovePhysicsObject(PhysicsObject* physicsObject)
{
	btCollisionObject* bulletCollisionObject = physicsObject->GetBulletCollisionObject();

	if (!bulletCollisionObject)
	{
		return;
	}

	decltype(physicsObjects_.begin()) physicsObjectIterator = physicsObjects_.begin();
	while(physicsObjectIterator != physicsObjects_.end())
	{
		if(*physicsObjectIterator == physicsObject)
		{
			physicsObjects_.erase(physicsObjectIterator);
			break;
		}

		++physicsObjectIterator;
	}

	dynamicsWorld_->removeCollisionObject(bulletCollisionObject);
}

void PhysicsWorld::AddPhysicsMovementComponent(PhysicsMovementComponent* physicsMovementComponent)
{
	physicsMovementComponents_.push_back(physicsMovementComponent);
	dynamicsWorld_->addAction(physicsMovementComponent->GetBulletKinematicCharacterController());
}

void PhysicsWorld::RemovePhysicsMovementComponent(PhysicsMovementComponent* physicsMovementComponent)
{
	if (!physicsMovementComponent)
	{
		return;
	}

	decltype(physicsMovementComponents_.cbegin()) physicsMovementComponentIterator = physicsMovementComponents_.cbegin();
	while(physicsMovementComponentIterator != physicsMovementComponents_.cend())
	{
		if(*physicsMovementComponentIterator == physicsMovementComponent)
		{
			physicsMovementComponents_.erase(physicsMovementComponentIterator);
			break;
		}

		++physicsMovementComponentIterator;
	}

	dynamicsWorld_->removeAction(physicsMovementComponent->GetBulletKinematicCharacterController());
}

bool PhysicsWorld::RaycastClosest(const RaycastData& raycastData, RaycastSingleResult& raycastClosest)
{
	btVector3 bulletFrom = PhysicsUtils::FromVector3ToBtVector3(raycastData.from);
	btVector3 bulletTo = PhysicsUtils::FromVector3ToBtVector3(raycastData.to);

	btCollisionWorld::ClosestRayResultCallback closestRayResultCallback(bulletFrom, bulletTo);

	closestRayResultCallback.m_collisionFilterGroup = (int)raycastData.collisionGroup;
	closestRayResultCallback.m_collisionFilterMask = (int)raycastData.collisionMask;

	dynamicsWorld_->rayTest(bulletFrom, bulletTo, closestRayResultCallback);

	if(closestRayResultCallback.hasHit())
	{
		raycastClosest.hitObject = (PhysicsObject*)closestRayResultCallback.m_collisionObject->getUserPointer();
		raycastClosest.hitFraction = closestRayResultCallback.m_closestHitFraction;
		raycastClosest.hitPosition = PhysicsUtils::FromBtVector3ToVector3(closestRayResultCallback.m_hitPointWorld);
		raycastClosest.hitNormal = PhysicsUtils::FromBtVector3ToVector3(closestRayResultCallback.m_hitNormalWorld);

		return true;
	}

	return false;
}

bool PhysicsWorld::RaycastAll(const RaycastData& raycastData, RaycastAllResult& raycastAllResult)
{
	btVector3 bulletFrom = PhysicsUtils::FromVector3ToBtVector3(raycastData.from);
	btVector3 bulletTo = PhysicsUtils::FromVector3ToBtVector3(raycastData.to);

	btCollisionWorld::AllHitsRayResultCallback allHitsRayResultCallback(bulletFrom, bulletTo);

	allHitsRayResultCallback.m_collisionFilterGroup = (int)raycastData.collisionGroup;
	allHitsRayResultCallback.m_collisionFilterMask = (int)raycastData.collisionMask;

	dynamicsWorld_->rayTest(bulletFrom, bulletTo, allHitsRayResultCallback);

	if(allHitsRayResultCallback.hasHit())
	{
		const int hitCount = allHitsRayResultCallback.m_collisionObjects.size();
		for (int hitIndex = 0; hitIndex < hitCount; hitIndex++)
		{
			raycastAllResult.hitResults.emplace_back(
				RaycastSingleResult(
					(PhysicsObject*)allHitsRayResultCallback.m_collisionObjects[hitIndex]->getUserPointer(),
					PhysicsUtils::FromBtVector3ToVector3(allHitsRayResultCallback.m_hitPointWorld[hitIndex]),
					PhysicsUtils::FromBtVector3ToVector3(allHitsRayResultCallback.m_hitNormalWorld[hitIndex]),
					allHitsRayResultCallback.m_hitFractions[hitIndex]
				)
			);
		}

		return true;
	}

	return false;
}

bool PhysicsWorld::SweepClosest(const SweepData& sweepData, RaycastSingleResult& result)
{
	GOKNAR_CORE_ASSERT(sweepData.collisionComponent->GetBulletCollisionShape()->isConvex());

	btConvexShape* bulletcollisionShape = (btConvexShape*)sweepData.collisionComponent->GetBulletCollisionShape();
	
	btTransform bulletFromTransform = PhysicsUtils::GetBulletTransform(sweepData.fromRotation, sweepData.fromPosition);
	btTransform bulletToTransform = PhysicsUtils::GetBulletTransform(sweepData.toRotation, sweepData.toPosition);

	btCollisionWorld::ClosestConvexResultCallback closestResultCallback(
		PhysicsUtils::FromVector3ToBtVector3(sweepData.fromPosition),
		PhysicsUtils::FromVector3ToBtVector3(sweepData.toPosition)
	);

	closestResultCallback.m_collisionFilterGroup = (int)sweepData.collisionGroup;
	closestResultCallback.m_collisionFilterMask = (int)sweepData.collisionMask;

	dynamicsWorld_->convexSweepTest(bulletcollisionShape, bulletFromTransform, bulletToTransform, closestResultCallback, sweepData.ccdPenetration);

	if(closestResultCallback.hasHit())
	{
		result.hitObject = (PhysicsObject*)closestResultCallback.m_hitCollisionObject->getUserPointer();
		result.hitFraction = closestResultCallback.m_closestHitFraction;
		result.hitPosition = PhysicsUtils::FromBtVector3ToVector3(closestResultCallback.m_hitPointWorld);
		result.hitNormal = PhysicsUtils::FromBtVector3ToVector3(closestResultCallback.m_hitNormalWorld);

		return true;
	}
	
	return false;
}