#include "TerrainGrassSpawner.h"

#include <algorithm>
#include <array>
#include <cmath>

#include "Terrain.h"

#include "Goknar/Components/InstancedStaticMeshComponent.h"
#include "Goknar/Engine.h"
#include "Goknar/Geometry/Box.h"
#include "Goknar/Helpers/AssetParser.h"
#include "Goknar/Managers/ResourceManager.h"
#include "Goknar/Materials/Material.h"
#include "Goknar/Materials/MaterialSerializer.h"
#include "Goknar/Math/Matrix.h"
#include "Goknar/Model/InstancedStaticMesh.h"
#include "Goknar/Model/InstancedStaticMeshInstance.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Physics/Components/NonMovingTriangleMeshCollisionComponent.h"
#include "Goknar/Physics/PhysicsWorld.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Helpers/SceneParser.h"


namespace
{
	constexpr float kGrassPlacementStep = 1.f;
	constexpr float kGrassMinimumScale = 0.75f;
	constexpr float kGrassMaximumScale = 1.5f;
	constexpr float kGrassMinimumRotationOnZ = 0.f;
	constexpr float kGrassMaximumRotationOnZ = 180.f;
	constexpr float kRaycastVerticalPadding = 100.f;

	size_t terrainGrassMeshIdentifier = 0;

	Vector3 TransformPoint(const Matrix& transform, const Vector3& point)
	{
		const Vector4 transformedPoint = transform * Vector4(point, 1.f);
		return Vector3(transformedPoint.x, transformedPoint.y, transformedPoint.z);
	}

	Box GetWorldAABB(const Box& localAABB, const Matrix& transform)
	{
		const Vector3& localMin = localAABB.GetMin();
		const Vector3& localMax = localAABB.GetMax();
		const std::array<Vector3, 8> localCorners =
		{
			Vector3(localMin.x, localMin.y, localMin.z),
			Vector3(localMin.x, localMin.y, localMax.z),
			Vector3(localMin.x, localMax.y, localMin.z),
			Vector3(localMin.x, localMax.y, localMax.z),
			Vector3(localMax.x, localMin.y, localMin.z),
			Vector3(localMax.x, localMin.y, localMax.z),
			Vector3(localMax.x, localMax.y, localMin.z),
			Vector3(localMax.x, localMax.y, localMax.z)
		};

		Box worldAABB(Vector3(MAX_FLOAT, MAX_FLOAT, MAX_FLOAT), Vector3(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT));
		for (const Vector3& localCorner : localCorners)
		{
			worldAABB.ExtendWRTPoint(TransformPoint(transform, localCorner), false);
		}

		worldAABB.CalculateSize();
		return worldAABB;
	}

	void ApplyMaterialPathsToInstancedStaticMesh(InstancedStaticMesh* instancedStaticMesh, const std::vector<std::string>& materialPaths)
	{
		if (!instancedStaticMesh)
		{
			return;
		}

		const auto& subMeshes = instancedStaticMesh->GetSubMeshes();
		for (size_t subMeshIndex = 0; subMeshIndex < subMeshes.size() && subMeshIndex < materialPaths.size(); ++subMeshIndex)
		{
			const std::string& materialPath = materialPaths[subMeshIndex];
			if (materialPath.empty() || !subMeshes[subMeshIndex])
			{
				continue;
			}

			Material* material = subMeshes[subMeshIndex]->GetMaterial();
			if (material)
			{
				MaterialSerializer::Deserialize(materialPath, material);
			}
		}
	}
}

TerrainGrassSpawner::TerrainGrassSpawner() : ObjectBase()
{
	instancedStaticMeshComponent_ = AddSubComponent<InstancedStaticMeshComponent>();

	StaticMesh* grassStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/Plants/SM_Grass.fbx");
	if (instancedStaticMeshComponent_ && grassStaticMesh)
	{
		InstancedStaticMesh* instancedGrassMesh = InstancedStaticMesh::CreateFromStaticMesh(
			grassStaticMesh,
			grassStaticMesh->GetPath() + "::TerrainGrassSpawner_" + std::to_string(terrainGrassMeshIdentifier++));
		if (instancedGrassMesh)
		{
			ApplyMaterialPathsToInstancedStaticMesh(instancedGrassMesh, AssetParser::GetMeshMaterialPaths("Meshes/Plants/SM_Grass.fbx"));
			instancedStaticMeshComponent_->SetMesh(instancedGrassMesh);
		}
	}

	SetName("TerrainGrassSpawner");
}

void TerrainGrassSpawner::BeginGame()
{
	ObjectBase::BeginGame();

	if (!instancedStaticMeshComponent_)
	{
		return;
	}

	InstancedStaticMeshInstance* meshInstance = instancedStaticMeshComponent_->GetMeshInstance();
	InstancedStaticMesh* instancedStaticMesh = meshInstance ? meshInstance->GetMesh() : nullptr;
	if (!instancedStaticMesh || instancedStaticMesh->GetInstanceCount() > 0)
	{
		return;
	}

	const std::vector<Terrain*> terrains = engine->GetObjectsOfType<Terrain>();
	if (terrains.empty() || !terrains[0])
	{
		return;
	}

	Terrain* terrain = terrains[0];
	NonMovingTriangleMeshCollisionComponent* terrainCollisionComponent = terrain->GetTerrainCollisionComponent();
	const StaticMesh* terrainMesh = terrainCollisionComponent ? terrainCollisionComponent->GetMesh() : nullptr;
	if (!terrainCollisionComponent || !terrainMesh)
	{
		return;
	}

	const Box worldTerrainAABB = GetWorldAABB(
		terrainMesh->GetAABB(),
		terrainCollisionComponent->GetComponentToWorldTransformationMatrix());
	const float raycastStartZ = worldTerrainAABB.GetMaxZ() + kRaycastVerticalPadding;
	const float raycastEndZ = worldTerrainAABB.GetMinZ() - kRaycastVerticalPadding;

	const int xCellCount = std::max(1, static_cast<int>(std::ceil(worldTerrainAABB.GetWidth() / kGrassPlacementStep)));
	const int yCellCount = std::max(1, static_cast<int>(std::ceil(worldTerrainAABB.GetDepth() / kGrassPlacementStep)));

	std::vector<Matrix> instanceTransformationMatrices;
	instanceTransformationMatrices.reserve(static_cast<size_t>(xCellCount) * static_cast<size_t>(yCellCount));

	for (float x = worldTerrainAABB.GetMinX(); x < worldTerrainAABB.GetMaxX(); x += kGrassPlacementStep)
	{
		const float maxSampleX = std::min(x + kGrassPlacementStep, worldTerrainAABB.GetMaxX());
		if (maxSampleX <= x)
		{
			continue;
		}

		for (float y = worldTerrainAABB.GetMinY(); y < worldTerrainAABB.GetMaxY(); y += kGrassPlacementStep)
		{
			const float maxSampleY = std::min(y + kGrassPlacementStep, worldTerrainAABB.GetMaxY());
			if (maxSampleY <= y)
			{
				continue;
			}

			const float sampleX = GoknarMath::GetRandom(x, maxSampleX);
			const float sampleY = GoknarMath::GetRandom(y, maxSampleY);

			RaycastData raycastData;
			raycastData.from = Vector3(sampleX, sampleY, raycastStartZ);
			raycastData.to = Vector3(sampleX, sampleY, raycastEndZ);
			raycastData.collisionGroup = CollisionGroup::All;
			raycastData.collisionMask = CollisionMask::BlockWorldStatic;

			RaycastSingleResult raycastResult;
			if (!engine->GetPhysicsWorld()->RaycastClosest(raycastData, raycastResult) || raycastResult.hitObject != terrain)
			{
				continue;
			}

			const float randomRotationOnZ = GoknarMath::GetRandom(kGrassMinimumRotationOnZ, kGrassMaximumRotationOnZ);
			const float randomUniformScale = GoknarMath::GetRandom(kGrassMinimumScale, kGrassMaximumScale);

			instanceTransformationMatrices.push_back(
				Matrix::GetTransformationMatrix(
					Quaternion::FromEulerDegrees(0.f, 0.f, randomRotationOnZ),
					raycastResult.hitPosition,
					Vector3(randomUniformScale)));
		}
	}

	instancedStaticMesh->SetInstanceTransformations(instanceTransformationMatrices);
	instancedStaticMesh->UpdateAllTransforms();
	SceneParser::SaveScene(engine->GetApplication()->GetMainScene(), ContentDir + "Scenes/Scene_Terrain_Generated");
}
