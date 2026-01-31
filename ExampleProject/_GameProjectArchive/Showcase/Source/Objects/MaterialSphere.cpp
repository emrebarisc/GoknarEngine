#include "MaterialSphere.h"

#include "Goknar/Engine.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/StaticMesh.h"

MaterialSphere::MaterialSphere()
{
    sphereMeshComponent_ = AddSubComponent<StaticMeshComponent>();
    
    StaticMesh* sphereStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Sphere.fbx");
    sphereMeshComponent_->SetMesh(sphereStaticMesh);
}