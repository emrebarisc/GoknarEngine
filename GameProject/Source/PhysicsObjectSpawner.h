#pragma once

#include <vector>

#include "Goknar/Core.h"
#include "Goknar/ObjectBase.h"

class PhysicsBox;

class GOKNAR_API PhysicsObjectSpawner : public ObjectBase
{
public:
    PhysicsObjectSpawner();
    ~PhysicsObjectSpawner();

    virtual void BeginGame() override;
    virtual void Tick(float deltaTime) override;

    Vector3 GetRandomPosition();
    Quaternion GetRandomRotation();
    void CreatePhysicsBox();
    void CreatePhysicsSphere();
    void CreatePhysicsCapsule();
    void CreateMonkey();
    void CreateMultipleCollisionComponentObject();
    void ThrowCannonBall();
    void SpawnStaticBoxes();

protected:

private:
    //Vector3 initialPosition_{ Vector3{-20.f, 20.f, 0.f} };
    Vector3 initialPosition_{ Vector3::ZeroVector };

    PhysicsBox* groundPhysicsBox_;
};