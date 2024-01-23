#include "pch.h"

#include "btBulletDynamicsCommon.h"

#include "PhysicsWorld.h"
#include "RigidBody.h"

PhysicsWorld::PhysicsWorld()
{
}

PhysicsWorld::~PhysicsWorld()
{

	if (m_dynamicsWorld)
	{
		int i;
		for (i = m_dynamicsWorld->getNumConstraints() - 1; i >= 0; i--)
		{
			m_dynamicsWorld->removeConstraint(m_dynamicsWorld->getConstraint(i));
		}
		for (i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
		{
			btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
			{
				delete body->getMotionState();
			}
			m_dynamicsWorld->removeCollisionObject(obj);
			delete obj;
		}
	}
	//delete collision shapes
	for (int j = 0; j < m_collisionShapes.size(); j++)
	{
		btCollisionShape* shape = m_collisionShapes[j];
		delete shape;
	}
	m_collisionShapes.clear();

	delete m_dynamicsWorld;
	m_dynamicsWorld = nullptr;

	delete m_solver;
	m_solver = nullptr;

	delete m_broadphase;
	m_broadphase = nullptr;

	delete m_dispatcher;
	m_dispatcher = nullptr;

	delete m_collisionConfiguration;
	m_collisionConfiguration = nullptr;
}

void PhysicsWorld::PreInit()
{
	///collision configuration contains default setup for memory, collision setup
	m_collisionConfiguration = new btDefaultCollisionConfiguration();
	//m_collisionConfiguration->setConvexConvexMultipointIterations();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);

	m_broadphase = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
	m_solver = solver;

	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
	m_dynamicsWorld->setGravity({ gravity_.x, gravity_.y, gravity_.z });
}

void PhysicsWorld::Init()
{
}

void PhysicsWorld::PostInit()
{
}

void PhysicsWorld::PhysicsTick(float deltaTime)
{
	m_dynamicsWorld->stepSimulation(deltaTime);

	for(RigidBody* rigidBody : rigidBodies_)
	{
		rigidBody->PhysicsTick(deltaTime);
	}
}

void PhysicsWorld::AddRigidBody(RigidBody* rigidBody)
{
	rigidBodies_.push_back(rigidBody);

	btRigidBody* bulletRigidBody = rigidBody->GetBulletRigidBody();
	m_dynamicsWorld->addRigidBody(bulletRigidBody);
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
	m_dynamicsWorld->removeRigidBody(bulletRigidBody);
}