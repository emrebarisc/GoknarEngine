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
#include "Components/CharacterMovementComponent.h"
#include "Components/CollisionComponent.h"
#include "Components/OverlappingTypes.h"

PhysicsWorld::PhysicsWorld()
{
	physicsDebugger_ = new PhysicsDebugger();
}

PhysicsWorld::~PhysicsWorld()
{
	if (dynamicsWorld_)
	{
		int i;
		for (i = dynamicsWorld_->getNumConstraints() - 1; i >= 0; i--)
		{
			dynamicsWorld_->removeConstraint(dynamicsWorld_->getConstraint(i));
		}
		for (i = dynamicsWorld_->getNumCollisionObjects() - 1; i >= 0; i--)
		{
			btCollisionObject* obj = dynamicsWorld_->getCollisionObjectArray()[i];
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
			{
				delete body->getMotionState();
			}
			dynamicsWorld_->removeCollisionObject(obj);
			delete obj;
		}
	}

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

	for(PhysicsObject* physicsObject : physicsObjects_)
	{
		physicsObject->PhysicsTick(deltaTime);
	}

	for(CharacterMovementComponent* characterMovementComponent : characterMovementComponents_)
	{
		// characterMovementComponent->PreStep(dynamicsWorld_);
		// characterMovementComponent->PlayerStep(dynamicsWorld_, deltaTime);
		characterMovementComponent->UpdateOwnerTransformation();
	}
}

void PhysicsWorld::OnOverlappingCollisionBegin(btPersistentManifold* const& monifoldPointPtr)
{
	const btCollisionObject* ghostObject1 = monifoldPointPtr->getBody0();
	const btCollisionObject* ghostObject2 = monifoldPointPtr->getBody1();

	PhysicsObject* collisionObject1 = physicsObjectMap_[ghostObject1];
	PhysicsObject* collisionObject2 = physicsObjectMap_[ghostObject2];

	GOKNAR_CORE_ASSERT(collisionObject1 && collisionObject2);
	
	CollisionComponent* collisionComponent1 = dynamic_cast<CollisionComponent*>(collisionObject1->GetRootComponent());
	CollisionComponent* collisionComponent2 = dynamic_cast<CollisionComponent*>(collisionObject2->GetRootComponent());

	GOKNAR_CORE_ASSERT(collisionComponent1 && collisionComponent2);

	Vector3 worldPositionOnA = PhysicsUtils::FromBtVector3ToVector3(monifoldPointPtr->getContactPoint(0).getPositionWorldOnA());
	Vector3 hitNormal = PhysicsUtils::FromBtVector3ToVector3(monifoldPointPtr->getContactPoint(0).m_normalWorldOnB);

	Vector3 worldPositionOnB = PhysicsUtils::FromBtVector3ToVector3(monifoldPointPtr->getContactPoint(0).getPositionWorldOnB());

	collisionComponent1->OverlapBegin(collisionObject2, collisionComponent2, worldPositionOnA, hitNormal);
	collisionComponent2->OverlapBegin(collisionObject1, collisionComponent1, worldPositionOnB, hitNormal);
}

void PhysicsWorld::OnOverlappingCollisionContinue(btManifoldPoint& monifoldPoint, const btCollisionObject* ghostObject1, const btCollisionObject* ghostObject2)
{
	PhysicsObject* collisionObject1 = physicsObjectMap_[ghostObject1];
	PhysicsObject* collisionObject2 = physicsObjectMap_[ghostObject2];

	GOKNAR_CORE_ASSERT(collisionObject1 && collisionObject2);

	CollisionComponent* collisionComponent1 = dynamic_cast<CollisionComponent*>(collisionObject1->GetRootComponent());
	CollisionComponent* collisionComponent2 = dynamic_cast<CollisionComponent*>(collisionObject2->GetRootComponent());

	GOKNAR_CORE_ASSERT(collisionComponent1 && collisionComponent2);

	Vector3 worldPositionOnA = PhysicsUtils::FromBtVector3ToVector3(monifoldPoint.getPositionWorldOnA());
	Vector3 hitNormal = PhysicsUtils::FromBtVector3ToVector3(monifoldPoint.m_normalWorldOnB);

	Vector3 worldPositionOnB = PhysicsUtils::FromBtVector3ToVector3(monifoldPoint.getPositionWorldOnB());

	collisionComponent1->OverlapContinue(collisionObject2, collisionComponent2, worldPositionOnA, hitNormal);
	collisionComponent2->OverlapContinue(collisionObject1, collisionComponent1, worldPositionOnB, hitNormal);
}

void PhysicsWorld::OnOverlappingCollisionEnd(btPersistentManifold* const& manifold)
{
	PhysicsObject* collisionObject1 = physicsObjectMap_[manifold->getBody0()];
	PhysicsObject* collisionObject2 = physicsObjectMap_[manifold->getBody1()];

	GOKNAR_CORE_ASSERT(collisionObject1 && collisionObject2);

	CollisionComponent* collisionComponent1 = dynamic_cast<CollisionComponent*>(collisionObject1->GetRootComponent());
	CollisionComponent* collisionComponent2 = dynamic_cast<CollisionComponent*>(collisionObject2->GetRootComponent());

	GOKNAR_CORE_ASSERT(collisionComponent1 && collisionComponent2);

	collisionComponent1->OverlapEnd(collisionObject2, collisionComponent2);
	collisionComponent2->OverlapEnd(collisionObject1, collisionComponent1);
};

void PhysicsWorld::AddRigidBody(RigidBody* rigidBody)
{
	btRigidBody* bulletRigidBody = rigidBody->GetBulletRigidBody();
	GOKNAR_ASSERT(physicsObjectMap_.find(bulletRigidBody) == physicsObjectMap_.end());
	physicsObjectMap_[bulletRigidBody] = rigidBody;

	dynamicsWorld_->addRigidBody(bulletRigidBody, (int)rigidBody->GetCollisionGroup(), (int)rigidBody->GetCollisionMask());
	physicsObjects_.push_back(rigidBody);
}

void PhysicsWorld::RemoveRigidBody(RigidBody* rigidBody)
{
	btRigidBody* bulletRigidBody = rigidBody->GetBulletRigidBody();

	PhysicsObjectMap::const_iterator relativePhysicsObject = physicsObjectMap_.find(bulletRigidBody);
	GOKNAR_ASSERT(relativePhysicsObject != physicsObjectMap_.end());
	physicsObjectMap_.erase(relativePhysicsObject);

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
	const btCollisionObject* bulletCollisionObject = dynamic_cast<const btCollisionObject*>(physicsObject->GetBulletCollisionObject());
	GOKNAR_ASSERT(physicsObjectMap_.find(bulletCollisionObject) == physicsObjectMap_.end());

	physicsObjectMap_[bulletCollisionObject] = physicsObject;

	dynamicsWorld_->addCollisionObject(physicsObject->GetBulletCollisionObject(), (int)physicsObject->GetCollisionGroup(), (int)physicsObject->GetCollisionMask());
}

void PhysicsWorld::RemovePhysicsObject(PhysicsObject* physicsObject)
{
	btCollisionObject* bulletCollisionObject = physicsObject->GetBulletCollisionObject();

	PhysicsObjectMap::const_iterator relativePhysicsObjectInMap = physicsObjectMap_.find(bulletCollisionObject);
	GOKNAR_ASSERT(relativePhysicsObjectInMap != physicsObjectMap_.end());
	physicsObjectMap_.erase(relativePhysicsObjectInMap);

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

void PhysicsWorld::AddCharacterMovementComponent(CharacterMovementComponent* characterMovementComponent)
{
	characterMovementComponents_.push_back(characterMovementComponent);
	dynamicsWorld_->addAction(characterMovementComponent->GetBulletKinematicCharacterController());
}

void PhysicsWorld::RemoveCharacterMovementComponent(CharacterMovementComponent* characterMovementComponent)
{
	decltype(characterMovementComponents_.begin()) characterIterator = characterMovementComponents_.begin();
	while(characterIterator != characterMovementComponents_.end())
	{
		if(*characterIterator == characterMovementComponent)
		{
			characterMovementComponents_.erase(characterIterator);
			break;
		}

		++characterIterator;
	}

	dynamicsWorld_->removeAction(characterMovementComponent->GetBulletKinematicCharacterController());
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
		raycastClosest.hitObject = (*physicsObjectMap_.find(closestRayResultCallback.m_collisionObject)).second;
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
					(*physicsObjectMap_.find(allHitsRayResultCallback.m_collisionObjects[hitIndex])).second,
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
		result.hitObject = (*physicsObjectMap_.find(closestResultCallback.m_hitCollisionObject)).second;
		result.hitFraction = closestResultCallback.m_closestHitFraction;
		result.hitPosition = PhysicsUtils::FromBtVector3ToVector3(closestResultCallback.m_hitPointWorld);
		result.hitNormal = PhysicsUtils::FromBtVector3ToVector3(closestResultCallback.m_hitNormalWorld);

		return true;
	}
	
	return false;
}