#include "Dancer.h"

#include "IO/ModelLoader.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/Materials/MaterialBase.h"

#include "Components/SkeletalMeshComponent.h"
#include "Managers/ResourceManager.h"
#include "Model/SkeletalMesh.h"
#include "Model/SkeletalMeshInstance.h"
#include "Physics/PhysicsWorld.h"

Dancer::Dancer() : ObjectBase()
{
	skeletalMesh_ = engine->GetResourceManager()->GetContent<SkeletalMesh>("Meshes/SkeletalMesh_Michelle.fbx");
	skeletalMeshComponent_ = AddSubComponent<SkeletalMeshComponent>();
	skeletalMeshComponent_->SetMesh(skeletalMesh_);
	skeletalMeshComponent_->SetRelativePosition(Vector3(0.f, 0.f, 0.f));
	skeletalMeshComponent_->SetRelativeRotation(Quaternion::FromEulerDegrees(Vector3(90.f, 0.f, 0.f)));
	skeletalMeshComponent_->SetRelativeScaling(Vector3{ 0.0125f });
	dynamic_cast<SkeletalMeshInstance*>(skeletalMeshComponent_->GetMeshInstance())->PlayAnimation("Armature|Layer0");
}

void Dancer::BeginGame()
{
	RaycastData raycastData;
	raycastData.from = Vector3{ -40.f, 40.f, 1000.f };
	raycastData.to = Vector3{ -40.f, 40.f, -1000.f };

	RaycastSingleResult raycastResult;

	if (engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult))
	{
		SetWorldPosition(raycastResult.hitPosition);
	}
}

void Dancer::Tick(float deltaTime)
{
}
