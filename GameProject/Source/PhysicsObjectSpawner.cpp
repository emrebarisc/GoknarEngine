#include "PhysicsObjectSpawner.h"

#include <random>

#include "Archer.h"
#include "Objects/PhysicsBox.h"
#include "Objects/PhysicsSphere.h"

#include "Engine.h"
#include "Game.h"
#include "Managers/InputManager.h"

PhysicsObjectSpawner::PhysicsObjectSpawner()
{
    SetIsTickable(true);

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
    //engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_PRESS, [this]() { CreatePhysicsBox(); });
    engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_PRESS, [this]() { ThrowPhysicsSphere(); });
}

void PhysicsObjectSpawner::Tick(float deltaTime)
{
    static float counter = 0.25f;

    if(counter < 0.f)
    {
        counter += 0.25f;
        CreatePhysicsBox();
    }

    counter -= deltaTime;
}

void PhysicsObjectSpawner::CreatePhysicsBox()
{
	std::random_device rd;
	std::uniform_real_distribution<float> positionDistX(-10.f, 10.f);
	std::uniform_real_distribution<float> positionDistY(-10.f, 10.f);
    float randomPositionX = positionDistX(rd);
    float randomPositionY = positionDistY(rd);

    PhysicsBox* physicsBox = new PhysicsBox();
    physicsBox->SetWorldPosition(initialPosition_ + Vector3{ randomPositionX, randomPositionY, 10.f });
}

void PhysicsObjectSpawner::ThrowPhysicsSphere()
{
    PhysicsSphere* physicsSphere = new PhysicsSphere();
    physicsSphere->SetWorldScaling(Vector3(0.2f));

    Game* game = dynamic_cast<Game*>(engine->GetApplication());

    Archer* archer = game->GetArcher();

    physicsSphere->SetWorldPosition(archer->GetWorldPosition() + archer->GetUpVector() + archer->GetForwardVector() * 0.25f);
    physicsSphere->SetLinearVelocity(50.f * (archer->GetForwardVector() * 0.85f + archer->GetUpVector() * 0.15f));
    //physicsSphere->ApplyForce(1000000.f * (archer->GetForwardVector() * 0.9f + archer->GetUpVector() * 0.1f));
}