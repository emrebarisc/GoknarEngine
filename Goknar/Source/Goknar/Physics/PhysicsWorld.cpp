#include "pch.h"

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

#include "Log.h"
#include "PhysicsUtils.h"
#include "PhysicsWorld.h"
#include "RigidBody.h"
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
}

void PhysicsWorld::PreInit()
{
	///collision configuration contains default setup for memory, collision setup
	collisionConfiguration_ = new btDefaultCollisionConfiguration();
	//m_collisionConfiguration->setConvexConvexMultipointIterations();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	dispatcher_ = new btCollisionDispatcher(collisionConfiguration_);

	broadphase_ = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	solver_ = solver;

	dynamicsWorld_ = new btDiscreteDynamicsWorld(dispatcher_, broadphase_, solver_, collisionConfiguration_);
	dynamicsWorld_->setGravity({ gravity_.x, gravity_.y, gravity_.z });
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

	for(RigidBody* rigidBody : rigidBodies_)
	{
		rigidBody->PhysicsTick(deltaTime);
	}

	for (size_t overlappingCollisionComponentIndex = 0; overlappingCollisionComponentIndex < overlappingCollisionComponentCount_; overlappingCollisionComponentIndex++)
	{
		GOKNAR_CORE_INFO("overlappingCollisionComponentCount_ {}", overlappingCollisionComponentCount_);

		btPairCachingGhostObject* ghostObject = dynamic_cast<btPairCachingGhostObject*>(overlappingCollisionComponents_[overlappingCollisionComponentIndex]);
		
		// Prepare for getting all the contact manifolds for one Overlapping Pair
		btManifoldArray manifoldArray;
		// Get all the Overlapping Pair
		btBroadphasePairArray& pairArray = ghostObject->getOverlappingPairCache()->getOverlappingPairArray();
		int numPairs = pairArray.size();

		for (int i=0;i < numPairs;i++)
		{
			GOKNAR_CORE_INFO("numPairs {}", numPairs);
			manifoldArray.clear();

			const btBroadphasePair& pair = pairArray[i];
			
			//unless we manually perform collision detection on this pair, the contacts are in the dynamics world paircache: 
			//The next line fetches the collision information for this Pair
			btBroadphasePair* collisionPair = dynamicsWorld_->getPairCache()->findPair(pair.m_pProxy0, pair.m_pProxy1);
			if (!collisionPair)
			{
				continue;
			}

			// Read out the all contact manifolds for this Overlapping Pair
			if (collisionPair->m_algorithm)
			{
				collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);
			}

			for (int j = 0; j < manifoldArray.size();j++)
			{
			GOKNAR_CORE_INFO("manifoldArray.size() {}", manifoldArray.size());
				btPersistentManifold* manifold = manifoldArray[j];

				// Check if the first object in the Pair is GhostObject or not.
				btScalar directionSign = manifold->getBody0() == ghostObject ? btScalar(-1.f) : btScalar(1.f);
				for (int p = 0; p < manifold->getNumContacts(); p++)
				{
					GOKNAR_CORE_INFO("manifold->getNumContacts {}", manifold->getNumContacts());

					const btManifoldPoint&pt = manifold->getContactPoint(p);
					if (pt.getDistance() < 0.f)
					{
						// Actually you can get the local information from this Overlapping pair, not just world Position
						const btVector3& ptA = pt.getPositionWorldOnA();
						const btVector3& ptB = pt.getPositionWorldOnB();
						const btVector3& normalOnB = pt.m_normalWorldOnB;
						/// work here

						GOKNAR_CORE_INFO("Overlap at {} with normal {}", PhysicsUtils::FromBtVector3ToVector3(ptA).ToString(), PhysicsUtils::FromBtVector3ToVector3(normalOnB).ToString());
					}
				}
			}
		}
	}
}

void PhysicsWorld::AddRigidBody(RigidBody* rigidBody)
{
	btRigidBody* bulletRigidBody = rigidBody->GetBulletRigidBody();
	GOKNAR_ASSERT(physicsObjectMap_.find(bulletRigidBody) == physicsObjectMap_.end());
	physicsObjectMap_[bulletRigidBody] = rigidBody;

	dynamicsWorld_->addRigidBody(bulletRigidBody, (int)rigidBody->GetCollisionGroup(), (int)rigidBody->GetCollisionMask());
	rigidBodies_.push_back(rigidBody);
}

void PhysicsWorld::RemoveRigidBody(RigidBody* rigidBody)
{
	btRigidBody* bulletRigidBody = rigidBody->GetBulletRigidBody();

	PhysicsObjectMap::const_iterator relativePhysicsObject = physicsObjectMap_.find(bulletRigidBody);
	GOKNAR_ASSERT(relativePhysicsObject != physicsObjectMap_.end());
	physicsObjectMap_.erase(relativePhysicsObject);

	decltype(rigidBodies_.begin()) rigidBodyIterator = rigidBodies_.begin();
	while(rigidBodyIterator != rigidBodies_.end())
	{
		if(*rigidBodyIterator == rigidBody)
		{
			rigidBodies_.erase(rigidBodyIterator);
			break;
		}

		++rigidBodyIterator;
	}

	dynamicsWorld_->removeRigidBody(bulletRigidBody);
}

void PhysicsWorld::AddOverlappingCollisionComponent(CollisionComponent* collisionComponent)
{
	++overlappingCollisionComponentCount_;
	overlappingCollisionComponents_.push_back(collisionComponent);
	dynamicsWorld_->addCollisionObject(collisionComponent->GetBulletCollisionObject());
	// m_dynamicsWorld->addCollisionObject(bulletCollisionObject_);
	// m_dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
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