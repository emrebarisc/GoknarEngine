#include "PhysicsObjectSpawner.h"

#include "Objects/PhysicsBox.h"

PhysicsObjectSpawner::PhysicsObjectSpawner()
{
    physicsBox_ = new PhysicsBox();
}

PhysicsObjectSpawner::~PhysicsObjectSpawner()
{
}

void PhysicsObjectSpawner::BeginGame()
{
    physicsBox_->SetWorldPosition(Vector3{0.f, 0.f, 100.f});
}