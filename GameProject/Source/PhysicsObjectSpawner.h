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

protected:

private:
    PhysicsBox* physicsBox_;
};