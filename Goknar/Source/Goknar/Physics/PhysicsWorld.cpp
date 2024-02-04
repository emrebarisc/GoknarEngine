#include "pch.h"

#include "btBulletDynamicsCommon.h"

#include "PhysicsWorld.h"
#include "RigidBody.h"

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
	//delete collision shapes
	for (int j = 0; j < collisionShapes_.size(); j++)
	{
		btCollisionShape* shape = collisionShapes_[j];
		delete shape;
	}
	collisionShapes_.clear();

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
}

void PhysicsWorld::AddRigidBody(RigidBody* rigidBody)
{
	rigidBodies_.push_back(rigidBody);

	btRigidBody* bulletRigidBody = rigidBody->GetBulletRigidBody();
	dynamicsWorld_->addRigidBody(bulletRigidBody, (int)rigidBody->GetCollisionGroup(), (int)rigidBody->GetCollisionMask());
}

void PhysicsWorld::RemoveRigidBody(RigidBody* rigidBody)
{
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

	btRigidBody* bulletRigidBody = rigidBody->GetBulletRigidBody();
	dynamicsWorld_->removeRigidBody(bulletRigidBody);
}