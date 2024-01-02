#include "MaterialSphereSpawner.h"

#include "Goknar/Material.h"
#include "Goknar/Components/StaticMeshComponent.h"

#include "Objects/MaterialSphere.h"

MaterialSphereSpawner::MaterialSphereSpawner()
{
    Vector3 offset{20.f, 20.f, 0.f};
    for (float y = 0; y < 10.f; y++)
    {
        for (float x = 0; x < 10.f; x++)
        {
            MaterialSphere* materialSphere = materialSpheres_.emplace_back(new MaterialSphere());
            materialSphere->SetWorldPosition(offset + Vector3{x * 2.f, y * 2.f, 1.f});
        }
    }
}

MaterialSphereSpawner::~MaterialSphereSpawner()
{
}

void MaterialSphereSpawner::BeginGame()
{
    for (float y = 0; y < 10.f; y++)
    {
        for (float x = 0; x < 10.f; x++)
        {
            MaterialSphere* materialSphere = materialSpheres_[y * 10 + x];

            StaticMeshComponent* materialSphereMeshComponent = materialSphere->GetSphereMeshComponent();
            StaticMeshInstance* staticMeshInstance = materialSphereMeshComponent->GetMeshInstance();
            StaticMesh* staticMesh = staticMeshInstance->GetMesh();

            Material* instanceMaterial = new Material(staticMesh->GetMaterial());
            instanceMaterial->SetDiffuseReflectance(Vector3{0.3725490196f, 0.f, 0.62745098039f});
            instanceMaterial->SetSpecularReflectance(Vector3{x / 9.f});
            instanceMaterial->SetPhongExponent(pow(2, y));
            staticMeshInstance->SetMaterial(instanceMaterial);
        }
    }
}