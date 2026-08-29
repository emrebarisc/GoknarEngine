#include "SkeletalMeshPerformanceTest.h" 

#include "Goknar/Components/SkeletalMeshComponent.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/MeshContainer.h"
#include "Goknar/Model/SkeletalMesh.h"

SkeletalMeshPerformanceTest::SkeletalMeshPerformanceTest()
{

}

void SkeletalMeshPerformanceTest::BeginGame()
{
	int gridSize = static_cast<int>(std::ceil(std::sqrt(objectCount)));

	float centerOffset = (gridSize - 1) * marginBetween / 2.0f;

	for (size_t objectIndex = 0; objectIndex < objectCount; ++objectIndex)
	{
		ObjectBase* newObject = new ObjectBase();
		SkeletalMeshComponent* skeletalMeshComponent = newObject->AddSubComponent<SkeletalMeshComponent>();
		skeletalMeshComponent->SetRelativeScaling(Vector3{ 0.01f });
		skeletalMeshComponent->SetRelativeRotation(Quaternion::FromEulerDegrees(90.f, 0.f, -90.f));
		skeletalMeshComponent->SetMesh(engine->GetResourceManager()->GetContent<SkeletalMesh>("Meshes/Characters/SK_DefaultCharacter.fbx"));
		skeletalMeshComponent->GetMeshInstance()->PlayAnimation("Armature|RifleIdle");

		size_t row = objectIndex / gridSize;
		size_t col = objectIndex % gridSize;

		float posX = (col * marginBetween) - centerOffset;
		float posY = (row * marginBetween) - centerOffset;

		newObject->SetWorldPosition(Vector3{ posX, posY, 0.0f });
	}
}
