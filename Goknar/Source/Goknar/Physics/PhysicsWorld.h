#ifndef __IPHYSICSWORLD_H__
#define __IPHYSICSWORLD_H__

#include <unordered_map>

#include "Core.h"
#include "Math/GoknarMath.h"
#include "Physics/PhysicsTypes.h"

#include "btBulletDynamicsCommon.h"

class btGhostObject;

class CollisionComponent;
class OverlappingCollisionPairCallback;
class RigidBody;

struct GOKNAR_API RaycastData
{
    Vector3 from;
    Vector3 to;
    CollisionGroup collisionGroup{ CollisionGroup::All };
    CollisionMask collisionMask{ CollisionMask::BlockAll };
};

struct GOKNAR_API RaycastClosestResult
{
    RigidBody* hitObject;
    Vector3 hitPosition;
    Vector3 hitNormal;
    float hitFraction;
};

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

    virtual void AddOverlappingCollisionComponent(CollisionComponent* collisionComponent);

    bool RaycastClosest(const RaycastData& raycastData, RaycastClosestResult& raycastClosest);
    //virtual bool RaycastAll(const RaycastData& raycastData, RaycastAllResult& raycastClosest);

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

    typedef std::unordered_map<btCollisionObject const*, RigidBody*> PhysicsObjectMap;
    PhysicsObjectMap physicsObjectMap_;

    typedef std::vector<CollisionComponent*> OverlappingCollisionComponents;
    OverlappingCollisionComponents overlappingCollisionComponents_;
    
    typedef std::unordered_map<btCollisionObject const*, CollisionComponent*> OverlappingCollisionComponentMap;
    OverlappingCollisionComponentMap overlappingCollisionComponentMap_;

private:
    void OnOverlappingCollisionPairAdded(btCollisionObject* ghostObject1, btCollisionObject* ghostObject2);
    void OnOverlappingCollisionPairRemoved(btCollisionObject* ghostObject1, btCollisionObject* ghostObject2);

    Vector3 gravity_{ Vector3{0.f, 0.f, -10.f} };

    btBroadphaseInterface* broadphase_{ nullptr };
    btCollisionDispatcher* dispatcher_{ nullptr };
    btConstraintSolver* solver_{ nullptr };
    btDefaultCollisionConfiguration* collisionConfiguration_{ nullptr };
    btDiscreteDynamicsWorld* dynamicsWorld_{ nullptr };
    OverlappingCollisionPairCallback* overlappingCollisionPairCallback_{ nullptr };
};

#endif