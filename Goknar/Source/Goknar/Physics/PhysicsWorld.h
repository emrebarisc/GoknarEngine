#ifndef __IPHYSICSWORLD_H__
#define __IPHYSICSWORLD_H__

#include "Core.h"
#include "Math/GoknarMath.h"

#include "btBulletDynamicsCommon.h"

class CollisionComponent;
class RigidBody;

class GOKNAR_API PhysicsWorld
{
public:
    PhysicsWorld();
    ~PhysicsWorld();

    virtual void PreInit();
    virtual void Init();
    virtual void PostInit();

    virtual void PhysicsTick(float deltaTime);

    virtual void AddRigidBody(RigidBody* rigidBody);
    virtual void RemoveRigidBody(RigidBody* rigidBody);

    virtual void AddCollision(CollisionComponent* collisionComponent) {}

    const Vector3& GetGravity() const
    {
        return gravity_;
    }

    void SetGravity(const Vector3& gravity)
    {
        gravity_ = gravity;
    }

protected:
    typedef std::vector<RigidBody*> RigidBodies;
    RigidBodies rigidBodies_;

private:
    Vector3 gravity_{ Vector3{0.f, 0.f, -10.f} };

    btAlignedObjectArray<btCollisionShape*> m_collisionShapes;
    btBroadphaseInterface* m_broadphase{ nullptr };
    btCollisionDispatcher* m_dispatcher{ nullptr };
    btConstraintSolver* m_solver{ nullptr };
    btDefaultCollisionConfiguration* m_collisionConfiguration{ nullptr };
    btDiscreteDynamicsWorld* m_dynamicsWorld{ nullptr };
};

#endif