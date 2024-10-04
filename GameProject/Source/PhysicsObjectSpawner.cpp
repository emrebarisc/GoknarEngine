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
#include "Physics/PhysicsWorld.h"

#include "Engine.h"
#include "Game.h"
#include "Components/StaticMeshComponent.h"
#include "Managers/InputManager.h"
#include "Model/StaticMeshInstance.h"

PhysicsObjectSpawner::PhysicsObjectSpawner()
{
    SetIsTickable(true);

    floor = new PhysicsBox();
    floor->SetMass(0.f);
    floor->SetWorldScaling({ 100.f, 100.f, 0.1f });
    Material* material = dynamic_cast<Material*>(floor->GetStaticMeshComponent()->GetMeshInstance()->GetMaterial());
    MaterialInstance* newMaterial = MaterialInstance::Create(material);

    newMaterial->SetBaseColor(Vector4{ 0.9f, 0.9f, 0.9f, 1.f });

    floor->GetStaticMeshComponent()->GetMeshInstance()->SetMaterial(newMaterial);
}

PhysicsObjectSpawner::~PhysicsObjectSpawner()
{
}

void PhysicsObjectSpawner::BeginGame()
{
    //engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_PRESS, [this]() { CreatePhysicsBox(); });
    engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_PRESS, [this]() { ThrowCannonBall(); });

    RaycastData raycastData;
    raycastData.from = Vector3(0.f, -8.f, 1000.f);
    raycastData.to = raycastData.from - Vector3(0.f, 0.f, 2000.f);

    RaycastSingleResult raycastResult;

    engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult);
    physicsBox = new PhysicsBox();
    physicsBox->SetWorldPosition(raycastResult.hitPosition + Vector3(0.f, 0.f, 2.f));
    physicsBox->SetLinearFactor(Vector3::ZeroVector);

    raycastData.from = Vector3(0.f, -4.f, 1000.f);
    raycastData.to = raycastData.from - Vector3(0.f, 0.f, 2000.f);
    engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult);
    PhysicsSphere* physicsSphere = new PhysicsSphere();
    physicsSphere->SetWorldPosition(raycastResult.hitPosition + Vector3(0.f, 0.f, 2.f));
    physicsSphere->SetLinearFactor(Vector3::ZeroVector);

    raycastData.from = Vector3(0.f, 0.f, 1000.f);
    raycastData.to = raycastData.from - Vector3(0.f, 0.f, 2000.f);
    engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult);
    PhysicsCapsule* physicsCapsule = new PhysicsCapsule();
    physicsCapsule->SetWorldPosition(raycastResult.hitPosition + Vector3(0.f, 0.f, 2.f));
    physicsCapsule->SetLinearFactor(Vector3::ZeroVector);

    raycastData.from = Vector3(0.f, 4.f, 1000.f);
    raycastData.to = raycastData.from - Vector3(0.f, 0.f, 2000.f);
    engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult);
    Monkey* monkey = new Monkey();
    monkey->SetWorldPosition(raycastResult.hitPosition + Vector3(0.f, 0.f, 2.f));
    monkey->SetLinearFactor(Vector3::ZeroVector);

    raycastData.from = Vector3(0.f, 8.f, 1000.f);
    raycastData.to = raycastData.from - Vector3(0.f, 0.f, 2000.f);
    engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult);
    MultipleCollisionComponentObject* multiple = new MultipleCollisionComponentObject();
    multiple->SetWorldPosition(raycastResult.hitPosition + Vector3(0.f, 0.f, 4.f));
    multiple->SetLinearFactor(Vector3::ZeroVector);


    raycastData.from = Vector3(-8.f, 8.f, 1000.f);
    raycastData.to = raycastData.from - Vector3(0.f, 0.f, 2000.f);
    engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult);
    floatingBox = new PhysicsBox();
    floatingBox->SetWorldPosition(raycastResult.hitPosition);
    floatingBox->SetWorldScaling(Vector3(4.f, 4.f, 0.1f));
    floatingBox->SetLinearFactor(Vector3::ZeroVector);
    floatingBox->SetAngularFactor(Vector3::ZeroVector);
    Material* material = dynamic_cast<Material*>(floatingBox->GetStaticMeshComponent()->GetMeshInstance()->GetMaterial());
    MaterialInstance* floatingBoxMaterial = MaterialInstance::Create(material);

    floatingBoxMaterial->SetBaseColor(Vector4{ 0.f, 1.f, 1.f, 1.f });

    floatingBox->GetStaticMeshComponent()->GetMeshInstance()->SetMaterial(floatingBoxMaterial);
}

void PhysicsObjectSpawner::Tick(float deltaTime)
{
    physicsBox->SetWorldRotation(Quaternion::FromEulerDegrees({ engine->GetElapsedTime() * 4.f }));
    physicsBox->SetWorldScaling(Vector3{ GoknarMath::Sin(engine->GetElapsedTime()) * 0.5f + 1.f });

    const Vector3 floatingBoxWorldPosition = floatingBox->GetWorldPosition();
    floatingBox->SetWorldPosition(Vector3{ floatingBoxWorldPosition.x, floatingBoxWorldPosition.y, -1.f + (GoknarMath::Cos(engine->GetElapsedTime()) * 0.5f + 1.f) * 2.f });
    return;

    static float counter = 1.f;

    if(counter < 0.f)
    {
        counter += 1.f;
        
        std::random_device randomDevice;
        std::uniform_int_distribution<int> randomObjectDist(0, 4);
        int randomObjectInt = randomObjectDist(randomDevice);

        CreatePhysicsSphere();

        //if(randomObjectInt == 0)
        //{
        //    CreatePhysicsBox();
        //}
        //else if(randomObjectInt == 1)
        //{
        //    CreatePhysicsSphere();
        //}
        //else if(randomObjectInt == 2)
        //{
        //    CreatePhysicsCapsule();
        //}
        //else if(randomObjectInt == 3)
        //{
        //    CreateMonkey();
        //}
        //else if(randomObjectInt == 4)
        //{
        //    CreateMultipleCollisionComponentObject();
        //}
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