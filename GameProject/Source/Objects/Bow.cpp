#include "Bow.h"

#include "Goknar/Engine.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Components/StaticMeshComponent.h"

#include "Components/SkeletalMeshComponent.h"

Bow::Bow() : ObjectBase()//StaticMeshObject()
{
	//StaticMesh* bowStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Bow.fbx");
	//staticMeshComponent->SetMesh(bowStaticMesh);
	//staticMeshComponent->SetRelativeRotation(Quaternion::FromEular(Vector3(-90.f, -90.f, 0.f)));

	skeletalMeshComponent_ = AddSubComponent<SkeletalMeshComponent>();
	SkeletalMesh* bowSkeletalMesh = engine->GetResourceManager()->GetContent<SkeletalMesh>("Meshes/SkeletalMesh_Bow.fbx");
	skeletalMeshComponent_->SetMesh(bowSkeletalMesh);
	skeletalMeshComponent_->SetRelativeRotation(Quaternion::FromEular(Vector3(-90.f, -90.f, 0.f)));
}