#include "pch.h"

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

#include "Engine.h"
#include "Log.h"
#include "PhysicsUtils.h"
#include "PhysicsWorld.h"
#include "RigidBody.h"
#include "Components/OverlappingTypes.h"
#include "Components/CollisionComponent.h"

PhysicsWorld::PhysicsWorld()
{
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

	delete dispatcher_;
	dispatcher_ = nullptr;

	delete collisionConfiguration_;
	collisionConfiguration_ = nullptr;

	delete overlappingCollisionPairCallback_;
	overlappingCollisionPairCallback_ = nullptr;
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

	broadphase_ = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	solver_ = solver;

	dynamicsWorld_ = new btDiscreteDynamicsWorld(dispatcher_, broadphase_, solver_, collisionConfiguration_);
	dynamicsWorld_->setGravity({ gravity_.x, gravity_.y, gravity_.z });

	gContactDestroyedCallback = OverlappingDestroyedCallback;
	//gContactProcessedCallback = OverlappingProcessedCallback;
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

	//overlappingCollisionComponents_.push_back(collisionComponent);
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

bool PhysicsWorld::RaycastClosest(const RaycastData& raycastData, RaycastClosestResult& raycastClosest)
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