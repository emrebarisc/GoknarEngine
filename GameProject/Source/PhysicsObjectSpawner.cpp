#include "PhysicsObjectSpawner.h"

#include <random>

#include "Archer.h"
#include "ArcherCharacter.h"
#include "Objects/PhysicsBox.h"
#include "Objects/PhysicsCapsule.h"
#include "Objects/PhysicsSphere.h"
#include "Objects/MultipleCollisionComponentObject.h"
#include "Objects/Monkey.h"
#include "Objects/CannonBall.h"

#include "Engine.h"
#include "Game.h"
#include "Components/StaticMeshComponent.h"
#include "Managers/InputManager.h"
#include "Model/StaticMeshInstance.h"

PhysicsObjectSpawner::PhysicsObjectSpawner()
{
    SetIsTickable(true);

    /*{
        PhysicsBox* wall = new PhysicsBox();
        wall->SetWorldScaling(Vector3{ 20.f, 20.f, 0.25f });
        wall->SetWorldPosition(Vector3{ 0.f, 0.f, -20.f });
        wall->SetMass(0.f);

        wall = new PhysicsBox();
        wall->SetWorldScaling(Vector3{ 20.f, 20.f, 0.25f });
        wall->SetWorldPosition(Vector3{ 0.f, 0.f, 20.f });
        wall->SetMass(0.f);
        StaticMeshComponent* groundBoxStaticMeshComponent = wall->GetStaticMeshComponent();
        StaticMeshInstance* wallStaticMeshInstance = groundBoxStaticMeshComponent->GetMeshInstance();
        Material* wallMaterial = wallStaticMeshInstance->GetMesh()->GetMaterial();
        MaterialInstance* groundMaterialInstance = MaterialInstance::Create(wallMaterial);
        groundMaterialInstance->SetBaseColor(Vector3{ 0.f, 1.f, 0.f });

        wallStaticMeshInstance->SetMaterial(groundMaterialInstance);

        wall = new PhysicsBox();
        wall->SetWorldScaling(Vector3{ 20.f, 20.f, 0.25f });
        wall->SetWorldPosition(Vector3{ 0.f, -20.f, 0.f });
        wall->SetWorldRotation(Quaternion::FromEulerDegrees(Vector3{ 90.f, 0.f, 0.f }));
        wall->SetMass(0.f);
        groundMaterialInstance = MaterialInstance::Create(wallMaterial);
        groundMaterialInstance->SetBaseColor(Vector3{ 0.f, 0.f, 1.f });
        groundBoxStaticMeshComponent = wall->GetStaticMeshComponent();
        wallStaticMeshInstance = groundBoxStaticMeshComponent->GetMeshInstance();
        wallStaticMeshInstance->SetMaterial(groundMaterialInstance);

        wall = new PhysicsBox();
        wall->SetWorldScaling(Vector3{ 20.f, 20.f, 0.25f });
        wall->SetWorldPosition(Vector3{ 0.f, 20.f, 0.f });
        wall->SetWorldRotation(Quaternion::FromEulerDegrees(Vector3{ 90.f, 0.f, 0.f }));
        wall->SetMass(0.f);
        groundMaterialInstance = MaterialInstance::Create(wallMaterial);
        groundMaterialInstance->SetBaseColor(Vector3{ 1.f, 1.f, 0.f });
        groundBoxStaticMeshComponent = wall->GetStaticMeshComponent();
        wallStaticMeshInstance = groundBoxStaticMeshComponent->GetMeshInstance();
        wallStaticMeshInstance->SetMaterial(groundMaterialInstance);

        wall = new PhysicsBox();
        wall->SetWorldScaling(Vector3{ 20.f, 20.f, 0.25f });
        wall->SetWorldPosition(Vector3{ -20.f, 0.f, 0.f });
        wall->SetWorldRotation(Quaternion::FromEulerDegrees(Vector3{ 90.f, 0.f, 90.f }));
        wall->SetMass(0.f);
        groundMaterialInstance = MaterialInstance::Create(wallMaterial);
        groundMaterialInstance->SetBaseColor(Vector3{ 1.f, 0.f, 1.f });
        groundBoxStaticMeshComponent = wall->GetStaticMeshComponent();
        wallStaticMeshInstance = groundBoxStaticMeshComponent->GetMeshInstance();
        wallStaticMeshInstance->SetMaterial(groundMaterialInstance);

        wall = new PhysicsBox();
        wall->SetWorldScaling(Vector3{ 20.f, 20.f, 0.25f });
        wall->SetWorldPosition(Vector3{ 20.f, 0.f, 0.f });
        wall->SetWorldRotation(Quaternion::FromEulerDegrees(Vector3{ 90.f, 0.f, 90.f }));
        wall->SetMass(0.f);
        groundMaterialInstance = MaterialInstance::Create(wallMaterial);
        groundMaterialInstance->SetBaseColor(Vector3{ 1.f, 1.f, 1.f });
        groundBoxStaticMeshComponent = wall->GetStaticMeshComponent();
        wallStaticMeshInstance = groundBoxStaticMeshComponent->GetMeshInstance();
        wallStaticMeshInstance->SetMaterial(groundMaterialInstance);
    }*/
}

PhysicsObjectSpawner::~PhysicsObjectSpawner()
{
}

void PhysicsObjectSpawner::BeginGame()
{
    //engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_PRESS, [this]() { CreatePhysicsBox(); });
    engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_PRESS, [this]() { ThrowCannonBall(); });

    // SpawnStaticBoxes();
}

void PhysicsObjectSpawner::Tick(float deltaTime)
{
    static float counter = 1.f;

    if(counter < 0.f)
    {
        counter += 1.f;
        
        std::random_device randomDevice;
        std::uniform_int_distribution<int> randomObjectDist(0, 4);
        int randomObjectInt = randomObjectDist(randomDevice);

        if(randomObjectInt == 0)
        {
            CreatePhysicsBox();
        }
        else if(randomObjectInt == 1)
        {
            CreatePhysicsSphere();
        }
        else if(randomObjectInt == 2)
        {
            CreatePhysicsCapsule();
        }
        else if(randomObjectInt == 3)
        {
            CreateMonkey();
        }
        else if(randomObjectInt == 4)
        {
            CreateMultipleCollisionComponentObject();
        }
    }

    counter -= deltaTime;
}

Vector3 PhysicsObjectSpawner::GetRandomPosition()
{
    std::random_device rd;
    std::uniform_real_distribution<float> positionDistX(-10.f, 10.f);
    std::uniform_real_distribution<float> positionDistY(-10.f, 10.f);
    float randomPositionX = positionDistX(rd);
    float randomPositionY = positionDistY(rd);

    return Vector3{ randomPositionX, randomPositionY, 10.f };
}

Quaternion PhysicsObjectSpawner::GetRandomRotation()
{
    std::random_device rd;
    std::uniform_real_distribution<float> positionDistX(0.f, PI);
    std::uniform_real_distribution<float> positionDistY(0.f, PI);
    std::uniform_real_distribution<float> positionDistZ(0.f, PI);
    float randomRotationX = positionDistX(rd);
    float randomRotationY = positionDistY(rd);
    float randomRotationZ = positionDistZ(rd);

    return Quaternion::FromEulerRadians(Vector3{ randomRotationX, randomRotationY, randomRotationZ });
}

void PhysicsObjectSpawner::CreatePhysicsBox()
{
    PhysicsBox* physicsBox = new PhysicsBox();
    physicsBox->SetWorldPosition(initialPosition_ + GetRandomPosition());
    physicsBox->SetWorldRotation(GetRandomRotation());
}

void PhysicsObjectSpawner::CreatePhysicsSphere()
{
    PhysicsSphere* physicsSphere = new PhysicsSphere();
    physicsSphere->SetWorldPosition(initialPosition_ + GetRandomPosition());
    physicsSphere->SetWorldRotation(GetRandomRotation());
}

void PhysicsObjectSpawner::CreatePhysicsCapsule()
{
    PhysicsCapsule* physicsCapsule = new PhysicsCapsule();
    physicsCapsule->SetWorldPosition(initialPosition_ + GetRandomPosition());
    physicsCapsule->SetWorldRotation(GetRandomRotation());
}

void PhysicsObjectSpawner::CreateMonkey()
{
    Monkey* monkey = new Monkey();
    monkey->SetWorldPosition(initialPosition_ + GetRandomPosition());
    monkey->SetWorldRotation(GetRandomRotation());
}

void PhysicsObjectSpawner::CreateMultipleCollisionComponentObject()
{
    MultipleCollisionComponentObject* multipleCollisionComponentObject = new MultipleCollisionComponentObject();
    multipleCollisionComponentObject->SetWorldPosition(initialPosition_ + GetRandomPosition());
    multipleCollisionComponentObject->SetWorldRotation(GetRandomRotation());
}
void PhysicsObjectSpawner::ThrowCannonBall()
{
    CannonBall* cannonBall = new CannonBall();
    cannonBall->SetWorldScaling(Vector3(0.2f));

    Game* game = dynamic_cast<Game*>(engine->GetApplication());

    ObjectBase* archer = game->GetPhysicsArcher();
    if (!archer)
    {
        archer = game->GetArcher();
    }

    cannonBall->SetWorldPosition(archer->GetWorldPosition() + archer->GetUpVector() + archer->GetForwardVector() * 0.25f);
    cannonBall->SetLinearVelocity(50.f * (archer->GetForwardVector() * 0.85f + archer->GetUpVector() * 0.15f));

    //cannonBall->ApplyForce(1000000.f * (archer->GetForwardVector() * 0.9f + archer->GetUpVector() * 0.1f));
}

void PhysicsObjectSpawner::SpawnStaticBoxes()
{
    constexpr float xOffset = 2.25f;
    constexpr float yOffset = 2.25f;
    for(int y = -20; y < 0; ++y)
    {
        for(int x = -20; x < 0; ++x)
        {
            PhysicsBox* physicsBox = new PhysicsBox();
            physicsBox->SetWorldPosition(initialPosition_ + Vector3{x + x * xOffset, y + y * yOffset, 10.f});
            physicsBox->SetLinearFactor(Vector3::ZeroVector);
        }
    }
}