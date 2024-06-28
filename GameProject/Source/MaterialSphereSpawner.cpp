#include "MaterialSphereSpawner.h"

#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Physics/PhysicsWorld.h"

#include "Objects/MaterialSphere.h"

constexpr int totalMaterialObjectSizeX = 5;
constexpr int totalMaterialObjectSizeY = 10;

MaterialSphereSpawner::MaterialSphereSpawner()
{
    for (float y = 0; y < totalMaterialObjectSizeY; y++)
    {
        for (float x = 0; x < totalMaterialObjectSizeX; x++)
        {
            MaterialSphere* materialSphere = materialSpheres_.emplace_back(new MaterialSphere());

            StaticMeshComponent* materialSphereMeshComponent = materialSphere->GetSphereMeshComponent();
            StaticMeshInstance* staticMeshInstance = materialSphereMeshComponent->GetMeshInstance();
            StaticMesh* staticMesh = staticMeshInstance->GetMesh();
            staticMeshInstance->SetIsCastingShadow(false);

            MaterialInstance* instanceMaterial = MaterialInstance::Create(staticMesh->GetMaterial());

            if (y < 3.f)
            {
                instanceMaterial->SetBaseColor(Vector3{ 0.3725490196f, 0.f, 0.62745098039f });
                instanceMaterial->SetSpecularReflectance(Vector3{ x / (totalMaterialObjectSizeX - 1) });
                instanceMaterial->SetPhongExponent(pow(2, y));
            }
            else if (y < 6.f)
            {
                if (y == 5.f)
                {
                    instanceMaterial->SetTranslucency(x / (totalMaterialObjectSizeX - 1));
                }
                instanceMaterial->SetBlendModel(MaterialBlendModel::Transparent);
                instanceMaterial->SetBaseColor(Vector4{ 0.3725490196f, 0.f, 0.62745098039f, x / (totalMaterialObjectSizeX - 1) });
            }
            else if (y < 9.f)
            {
                instanceMaterial->SetBaseColor(Vector4{ 0.3725490196f, 0.f, 0.62745098039f, 1.f });
                instanceMaterial->SetTranslucency(x / (totalMaterialObjectSizeX - 1));
            }
            else if (y < 10.f)
            {
                instanceMaterial->SetBaseColor(Vector4{ 1.f, 0.f, 0.f, 1.f });
                instanceMaterial->SetEmmisiveColor(Vector3{ 0.f, 0.f, x / (totalMaterialObjectSizeX - 1) });
                instanceMaterial->SetBlendModel(MaterialBlendModel::Transparent);
            }

            staticMeshInstance->SetMaterial(instanceMaterial);
        }
    }
}

MaterialSphereSpawner::~MaterialSphereSpawner()
{
}

void MaterialSphereSpawner::BeginGame()
{
    Vector3 offset{ -60.f, -40.f, 0.f };
    for (float y = 0; y < totalMaterialObjectSizeY; y++)
    {
        for (float x = 0; x < totalMaterialObjectSizeX; x++)
        {
            Vector3 position = offset + Vector3{ x * 2.f, y * 2.f, 1.f };

            RaycastData raycastData;
            raycastData.from = position + Vector3{ 0.f, 0.f, 1000.f };
            raycastData.to = position - Vector3{ 0.f, 0.f, 1000.f };

            RaycastSingleResult raycastResult;

            if (engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult))
            {
                MaterialSphere* materialSphere = materialSpheres_[totalMaterialObjectSizeX * y + x];
                materialSphere->SetWorldPosition(raycastResult.hitPosition + Vector3{ 0.f, 0.f, 1.f });
            }
        }
    }
}