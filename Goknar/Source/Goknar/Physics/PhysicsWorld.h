#ifndef __IPHYSICSWORLD_H__
#define __IPHYSICSWORLD_H__

#include <unordered_map>

#include "Core.h"
#include "Math/GoknarMath.h"
#include "Physics/PhysicsTypes.h"

#include "btBulletDynamicsCommon.h"

class btGhostObject;
class btGhostPairCallback;

class Character;
class CollisionComponent;
class CharacterMovementComponent;
class OverlappingCollisionPairCallback;
class PhysicsObject;
class RigidBody;

struct GOKNAR_API RaycastData
{
    Vector3 from;
    Vector3 to;
    CollisionGroup collisionGroup{ CollisionGroup::All };
    CollisionMask collisionMask{ CollisionMask::BlockAndOverlapAll };
};

struct GOKNAR_API RaycastClosestResult
{
    PhysicsObject* hitObject;
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

    virtual void AddPhysicsObject(PhysicsObject* physicsObject);
    virtual void RemovePhysicsObject(PhysicsObject* physicsObject);
    
    virtual void AddCharacterMovementComponent(CharacterMovementComponent* characterMovementComponent);
    virtual void RemoveCharacterMovementComponent(CharacterMovementComponent* characterMovementComponent);

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

    void OnOverlappingCollisionBegin(btPersistentManifold* const& manifold);
    void OnOverlappingCollisionContinue(btManifoldPoint& monifoldPoint, const btCollisionObject* ghostObject1, const btCollisionObject* ghostObject2);
    void OnOverlappingCollisionEnd(btPersistentManifold* const& manifold);

    btDiscreteDynamicsWorld* GetBulletPhysicsWorld() const
    {
        return dynamicsWorld_;
    }

protected:
    typedef std::vector<PhysicsObject*> PhysicsObjectVector;
    PhysicsObjectVector physicsObjects_;
    
    typedef std::vector<CharacterMovementComponent*> CharacterMovementComponentVector;
    CharacterMovementComponentVector characterMovementComponents_;

    typedef std::unordered_map<btCollisionObject const*, PhysicsObject*> PhysicsObjectMap;
    PhysicsObjectMap physicsObjectMap_;

    typedef std::vector<CollisionComponent*> OverlappingCollisionComponents;
    OverlappingCollisionComponents overlappingCollisionComponents_;

private:

    Vector3 gravity_{ Vector3{0.f, 0.f, -10.f} };

    btGhostPairCallback* ghostPairCallback_{ nullptr };
    btBroadphaseInterface* broadphase_{ nullptr };
    btCollisionDispatcher* dispatcher_{ nullptr };
    btDispatcherInfo* dispatcherInfo_{ nullptr };
    btConstraintSolver* solver_{ nullptr };
    btDefaultCollisionConfiguration* collisionConfiguration_{ nullptr };
    btDiscreteDynamicsWorld* dynamicsWorld_{ nullptr };
    OverlappingCollisionPairCallback* overlappingCollisionPairCallback_{ nullptr };
};

#endif