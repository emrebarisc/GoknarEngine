#include "MaterialSphereSpawner.h"

#include "Goknar/Material.h"
#include "Goknar/Components/StaticMeshComponent.h"

#include "Objects/MaterialSphere.h"

MaterialSphereSpawner::MaterialSphereSpawner()
{
    Vector3 offset{20.f, 20.f, 0.f};
    for (float y = 0; y < 5.f; y++)
    {
        for (float x = 0; x < 5.f; x++)
        {
            MaterialSphere* materialSphere = new MaterialSphere();
            StaticMeshComponent* materialSphereMeshComponent = materialSphere->GetSphereMeshComponent();
            StaticMesh* staticMesh = materialSphereMeshComponent->GetMeshInstance()->GetMesh();
            Material* material = staticMesh->GetMaterial();
            material->SetSpecularReflectance(Vector3{5.f / x});
            material->SetPhongExponent(5.f / y);

            materialSphere->SetWorldPosition(offset + Vector3{x * 3.f, y * 3.f, 1.f});
        }
    }
}

MaterialSphereSpawner::~MaterialSphereSpawner()
{

}

void MaterialSphereSpawner::BeginGame()
{
}