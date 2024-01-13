#include "MaterialSphereSpawner.h"

#include "Goknar/Materials/MaterialBase.h"
#include "Goknar/Components/StaticMeshComponent.h"

#include "Objects/MaterialSphere.h"

MaterialSphereSpawner::MaterialSphereSpawner()
{
}

MaterialSphereSpawner::~MaterialSphereSpawner()
{
}

void MaterialSphereSpawner::BeginGame()
{
    Vector3 offset{20.f, 20.f, 0.f};
    for (float y = 0; y < 10.f; y++)
    {
        for (float x = 0; x < 10.f; x++)
        {
            MaterialSphere* materialSphere = materialSpheres_.emplace_back(new MaterialSphere());
            materialSphere->SetWorldPosition(offset + Vector3{x * 2.f, y * 2.f, 1.f});
            
            StaticMeshComponent* materialSphereMeshComponent = materialSphere->GetSphereMeshComponent();
            StaticMeshInstance* staticMeshInstance = materialSphereMeshComponent->GetMeshInstance();
            StaticMesh* staticMesh = staticMeshInstance->GetMesh();

            MaterialInstance* instanceMaterial = MaterialInstance::Create(staticMesh->GetMaterial());

            if(y < 5.f)
            {
                instanceMaterial->SetDiffuseReflectance(Vector3{0.3725490196f, 0.f, 0.62745098039f});
                instanceMaterial->SetSpecularReflectance(Vector3{x / 9.f});
                instanceMaterial->SetPhongExponent(pow(2, y));
            }
            else
            {
                instanceMaterial->SetBlendModel(MaterialBlendModel::Transparent);
                instanceMaterial->SetDiffuseReflectance(Vector4{});
            }
            staticMeshInstance->SetMaterial(instanceMaterial);
        }
    }
}