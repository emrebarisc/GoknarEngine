#include "pch.h"

#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

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

	overlappingCollisionPairCallback_ = new OverlappingCollisionPairCallback();
	overlappingCollisionPairCallback_->OnAddOverlappingPair = Delegate<void(btCollisionObject*, btCollisionObject*)>::create<PhysicsWorld, &PhysicsWorld::OnOverlappingCollisionPairAdded>(this);
	overlappingCollisionPairCallback_->OnRemoveOverlappingPair = Delegate<void(btCollisionObject*, btCollisionObject*)>::create<PhysicsWorld, &PhysicsWorld::OnOverlappingCollisionPairRemoved>(this);
	broadphase_->getOverlappingPairCache()->setInternalGhostPairCallback(overlappingCollisionPairCallback_);
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
}

void PhysicsWorld::OnOverlappingCollisionPairAdded(btCollisionObject* ghostObject1, btCollisionObject* ghostObject2)
{
	ObjectBase* collisionObject1 = nullptr;
	if(overlappingCollisionComponentMap_.find(ghostObject1) != overlappingCollisionComponentMap_.end())
	{
		collisionObject1 = overlappingCollisionComponentMap_[ghostObject1]->GetOwner();
	}
	else if(physicsObjectMap_.find(ghostObject1) != physicsObjectMap_.end())
	{
		collisionObject1 = physicsObjectMap_[ghostObject1];
	}

	ObjectBase* collisionObject2 = nullptr;
	if(overlappingCollisionComponentMap_.find(ghostObject2) != overlappingCollisionComponentMap_.end())
	{
		collisionObject2 = overlappingCollisionComponentMap_[ghostObject2]->GetOwner();
	}
	else if(physicsObjectMap_.find(ghostObject2) != physicsObjectMap_.end())
	{
		collisionObject2 = physicsObjectMap_[ghostObject2];
	}
	GOKNAR_CORE_ASSERT(collisionObject1 && collisionObject2);

	GOKNAR_CORE_INFO("OnAddOverlappingPair {}-{}", collisionObject1->GetName(), collisionObject2->GetName());
};

void PhysicsWorld::OnOverlappingCollisionPairRemoved(btCollisionObject* ghostObject1, btCollisionObject* ghostObject2)
{
	ObjectBase* collisionObject1 = nullptr;
	if(overlappingCollisionComponentMap_.find(ghostObject1) != overlappingCollisionComponentMap_.end())
	{
		collisionObject1 = overlappingCollisionComponentMap_[ghostObject1]->GetOwner();
	}
	else if(physicsObjectMap_.find(ghostObject1) != physicsObjectMap_.end())
	{
		collisionObject1 = physicsObjectMap_[ghostObject1];
	}

	ObjectBase* collisionObject2 = nullptr;
	if(overlappingCollisionComponentMap_.find(ghostObject2) != overlappingCollisionComponentMap_.end())
	{
		collisionObject2 = overlappingCollisionComponentMap_[ghostObject2]->GetOwner();
	}
	else if(physicsObjectMap_.find(ghostObject2) != physicsObjectMap_.end())
	{
		collisionObject2 = physicsObjectMap_[ghostObject2];
	}
	GOKNAR_CORE_ASSERT(collisionObject1 && collisionObject2);

	GOKNAR_CORE_INFO("OnRemoveOverlappingPair {}-{}", collisionObject1->GetName(), collisionObject2->GetName());
};

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
	btGhostObject* bulletGhostObject = dynamic_cast<btGhostObject*>(collisionComponent->GetBulletCollisionObject());
	GOKNAR_ASSERT(overlappingCollisionComponentMap_.find(bulletGhostObject) == overlappingCollisionComponentMap_.end());
	overlappingCollisionComponentMap_[bulletGhostObject] = collisionComponent;

	overlappingCollisionComponents_.push_back(collisionComponent);
	dynamicsWorld_->addCollisionObject(collisionComponent->GetBulletCollisionObject(), (int)collisionComponent->GetCollisionGroup(), (int)collisionComponent->GetCollisionMask());
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