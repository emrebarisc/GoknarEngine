#include "MaterialSphereSpawner.h"

#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Physics/PhysicsWorld.h"

#include "Objects/MaterialSphere.h"

MaterialSphereSpawner::MaterialSphereSpawner()
{
}

MaterialSphereSpawner::~MaterialSphereSpawner()
{
}

void MaterialSphereSpawner::BeginGame()
{
    Vector3 offset{-60.f, 0.f, 0.f};
    for (float y = 0; y < 9.f; y++)
    {
        for (float x = 0; x < 5.f; x++)
        {

            Vector3 position = offset + Vector3{ x * 2.f, y * 2.f, 1.f };

            RaycastData raycastData;
            raycastData.from = position + Vector3{ 0.f, 0.f, 1000.f };
            raycastData.to = position - Vector3{ 0.f, 0.f, 1000.f };

            RaycastClosestResult raycastResult;

            if (engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult))
            {
                MaterialSphere* materialSphere = materialSpheres_.emplace_back(new MaterialSphere());
                materialSphere->SetWorldPosition(raycastResult.hitPosition + Vector3{ 0.f, 0.f, 1.f });

                StaticMeshComponent* materialSphereMeshComponent = materialSphere->GetSphereMeshComponent();
                StaticMeshInstance* staticMeshInstance = materialSphereMeshComponent->GetMeshInstance();
                StaticMesh* staticMesh = staticMeshInstance->GetMesh();

                MaterialInstance* instanceMaterial = MaterialInstance::Create(staticMesh->GetMaterial());

                if (y < 3.f)
                {
                    instanceMaterial->SetDiffuseReflectance(Vector3{ 0.3725490196f, 0.f, 0.62745098039f });
                    instanceMaterial->SetSpecularReflectance(Vector3{ x / 9.f });
                    instanceMaterial->SetPhongExponent(pow(2, y));
                }
                else if (y < 6.f)
                {
                    if (y == 5.f)
                    {
                        instanceMaterial->SetTranslucency(x / 5.f);
                    }
                    instanceMaterial->SetBlendModel(MaterialBlendModel::Transparent);
                    instanceMaterial->SetDiffuseReflectance(Vector4{ 0.3725490196f, 0.f, 0.62745098039f, x / 5.f });
                }
                else if (y < 9.f)
                {
                    instanceMaterial->SetDiffuseReflectance(Vector4{ 0.3725490196f, 0.f, 0.62745098039f, 1.f });
                    instanceMaterial->SetTranslucency(x / 5.f);
                    staticMeshInstance->SetIsCastingShadow(false);
                }
                staticMeshInstance->SetMaterial(instanceMaterial);
            }
        }
    }
}