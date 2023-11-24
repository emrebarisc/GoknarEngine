#include "RandomGrassSpawner.h"

#include "Goknar/Engine.h"
#include "Goknar/Components/StaticMeshComponent.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Material.h"

#include <random> 

RandomGrassSpawner::RandomGrassSpawner()
{
	SetIsTickable(false);
		const Vector3 initialPosition = Vector3{20.f, 20.f, 0.f};
	constexpr float xDiff = 1.f;
	constexpr float yDiff = 1.f;

	float minPositionDifferenceX = 0.f;
	float maxPositionDifferenceX = 0.8f;

	float minPositionDifferenceY = 0.f;
	float maxPositionDifferenceY = 0.8f;

	float minScale = 0.75f;
	float maxScale = 1.25f;

	float minRotationOnZ = 0.f;
	float maxRotationOnZ = 3.1415f;

	StaticMesh* grassStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_Grass.fbx");
	grassStaticMesh->GetMaterial()->SetShadingModel(MaterialShadingModel::TwoSided);
	grassStaticMesh->GetMaterial()->SetBlendModel(MaterialBlendModel::Masked);

	std::random_device rd;
	std::uniform_real_distribution<float> positionDistX(0.f, 1.f);
	std::uniform_real_distribution<float> positionDistY(0.f, 1.f);
	std::uniform_real_distribution<float> rotationDist(0.f, 1.f);
	std::uniform_real_distribution<float> scalingDist(0.f, 1.f);

	for(int y = 0; y < 16; ++y)
	{
		for(int x = 0; x < 16; ++x)
		{
			float randomPositionX = GoknarMath::Lerp(minPositionDifferenceX, maxPositionDifferenceX, positionDistX(rd));
			float randomPositionY = GoknarMath::Lerp(minPositionDifferenceY, maxPositionDifferenceY, positionDistY(rd));
			Quaternion randomRotationOnZ = Quaternion::FromEulerRadians(Vector3{0.f, 0.f, GoknarMath::Lerp(minRotationOnZ, maxRotationOnZ, rotationDist(rd))}) ;
			Vector3 randomScaling = Vector3{ GoknarMath::Lerp(minScale, maxScale, scalingDist(rd)) };

			ObjectBase* grass = new ObjectBase();
			StaticMeshComponent* grassComponent = grass->AddSubComponent<StaticMeshComponent>();
			grassComponent->SetMesh(grassStaticMesh);
			grass->SetWorldPosition(initialPosition + Vector3{x * xDiff + randomPositionX, y * yDiff + randomPositionY, 0.f} );
			grass->SetWorldRotation(randomRotationOnZ);
			grass->SetWorldScaling(randomScaling);
		}
	}
}

void RandomGrassSpawner::BeginGame()
{

}

void RandomGrassSpawner::Tick(float deltaTime)
{

}