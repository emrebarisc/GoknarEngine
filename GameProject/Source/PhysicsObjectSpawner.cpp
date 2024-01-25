#include "PhysicsObjectSpawner.h"

#include <random>

#include "Objects/PhysicsBox.h"

#include "Engine.h"
#include "Managers/InputManager.h"

PhysicsObjectSpawner::PhysicsObjectSpawner()
{
    groundPhysicsBox_ = new PhysicsBox();
    groundPhysicsBox_->SetWorldPosition(initialPosition_);
    groundPhysicsBox_->SetWorldScaling(Vector3{ 10.f, 10.f, 0.25f });
    groundPhysicsBox_->SetMass(0.f);
}

PhysicsObjectSpawner::~PhysicsObjectSpawner()
{
}

void PhysicsObjectSpawner::BeginGame()
{
    engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_PRESS, [this]() { CreatePhysicsBox(); });
}

void PhysicsObjectSpawner::CreatePhysicsBox()
{
	std::random_device rd;
	std::uniform_real_distribution<float> positionDistX(-10.f, 10.f);
	std::uniform_real_distribution<float> positionDistY(-10.f, 10.f);
    float randomPositionX = positionDistX(rd);
    float randomPositionY = positionDistY(rd);

    PhysicsBox* physicsBox = new PhysicsBox();
    physicsBox->SetMass(100.f);
    physicsBox->SetWorldPosition(initialPosition_ + Vector3{ randomPositionX, randomPositionY, 10.f });
}