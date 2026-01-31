#include "Bow.h"

#include "Goknar/Components/SkeletalMeshComponent.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/StaticMesh.h"

Bow::Bow() : ObjectBase()
{
	skeletalMeshComponent_ = AddSubComponent<SkeletalMeshComponent>();
	SkeletalMesh* bowSkeletalMesh = engine->GetResourceManager()->GetContent<SkeletalMesh>("Meshes/SkeletalMesh_Bow.fbx");
	skeletalMeshComponent_->SetMesh(bowSkeletalMesh);
	skeletalMeshComponent_->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3(-90.f, -90.f, 0.f)));
}