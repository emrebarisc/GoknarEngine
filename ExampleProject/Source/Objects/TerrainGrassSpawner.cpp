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
#include "Goknar/Physics/RigidBody.h"
#include "Goknar/Physics/Components/CapsuleCollisionComponent.h"


namespace
{
	constexpr float kGrassPlacementStep = 0.5f;
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

	int Hash(int x, int y, int seed) {
		unsigned int n = (x * 374761393U) ^ (y * 668265263U) ^ (seed * 1274126177U);
		n = (n ^ (n >> 13)) * 1274126177U;
		return n ^ (n >> 16);
	}

	float Fade(float t) { return t * t * t * (t * (t * 6.f - 15.f) + 10.f); }
	float Lerp(float t, float a, float b) { return a + t * (b - a); }
	float Grad2D(int hash, float x, float y) {
		switch (hash & 3) {
		case 0: return x + y;
		case 1: return -x + y;
		case 2: return x - y;
		case 3: return -x - y;
		}
		return 0.f;
	}

	float Perlin(float x, float y, int seed) {
		int xi = (int)std::floor(x);
		int yi = (int)std::floor(y);
		float xf = x - xi;
		float yf = y - yi;

		float u = Fade(xf);
		float v = Fade(yf);

		int aa = Hash(xi, yi, seed);
		int ba = Hash(xi + 1, yi, seed);
		int ab = Hash(xi, yi + 1, seed);
		int bb = Hash(xi + 1, yi + 1, seed);

		float x1 = Lerp(u, Grad2D(aa, xf, yf), Grad2D(ba, xf - 1.f, yf));
		float x2 = Lerp(u, Grad2D(ab, xf, yf - 1.f), Grad2D(bb, xf - 1.f, yf - 1.f));

		return (Lerp(v, x1, x2) + 1.0f) / 2.0f;
	}

	class TreeObject : public RigidBody
	{
	public:
		TreeObject() : RigidBody()
		{
			SetCollisionGroup(CollisionGroup::WorldDynamicBlock);
			SetCollisionMask(CollisionMask::BlockAndOverlapAll);
			SetMass(0.f);

			capsule_ = AddSubComponent<CapsuleCollisionComponent>();
			capsule_->SetRelativePosition(Vector3(0.f, 0.f, 2.f));
			capsule_->SetRadius(0.2f);
			capsule_->SetHeight(4.f);
		}
	private:
		CapsuleCollisionComponent* capsule_{ nullptr };
	};
}

TerrainGrassSpawner::TerrainGrassSpawner() : ObjectBase()
{
	grassInstancedStaticMeshComponent_ = AddSubComponent<InstancedStaticMeshComponent>();

	StaticMesh* grassStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/Plants/SM_Grass.fbx");
	if (grassInstancedStaticMeshComponent_ && grassStaticMesh)
	{
		InstancedStaticMesh* instancedGrassMesh = InstancedStaticMesh::CreateFromStaticMesh(
			grassStaticMesh,
			grassStaticMesh->GetPath() + "::TerrainGrassSpawner_" + std::to_string(terrainGrassMeshIdentifier++));
		if (instancedGrassMesh)
		{
			ApplyMaterialPathsToInstancedStaticMesh(instancedGrassMesh, AssetParser::GetMeshMaterialPaths("Meshes/Plants/SM_Grass.fbx"));
			grassInstancedStaticMeshComponent_->SetMesh(instancedGrassMesh);
		}
	}

	treeInstancedStaticMeshComponent_ = AddSubComponent<InstancedStaticMeshComponent>();

	StaticMesh* treeStaticMesh = engine->GetResourceManager()->GetContent<StaticMesh>("Meshes/SM_TreeBirch_01_Red.FBX");
	if (treeInstancedStaticMeshComponent_ && treeStaticMesh)
	{
		InstancedStaticMesh* instancedTreeMesh = InstancedStaticMesh::CreateFromStaticMesh(
			treeStaticMesh,
			treeStaticMesh->GetPath() + "::TerrainTreeSpawner_" + std::to_string(terrainGrassMeshIdentifier++));
		if (instancedTreeMesh)
		{
			ApplyMaterialPathsToInstancedStaticMesh(instancedTreeMesh, AssetParser::GetMeshMaterialPaths("Meshes/SM_TreeBirch_01_Red.FBX"));
			treeInstancedStaticMeshComponent_->SetMesh(instancedTreeMesh);
		}
	}

	SetName("TerrainGrassSpawner");
}

void TerrainGrassSpawner::BeginGame()
{
	ObjectBase::BeginGame();

	if (!grassInstancedStaticMeshComponent_ || !treeInstancedStaticMeshComponent_)
	{
		return;
	}

	InstancedStaticMeshInstance* grassMeshInstance = grassInstancedStaticMeshComponent_->GetMeshInstance();
	InstancedStaticMesh* instancedGrassMesh = grassMeshInstance ? grassMeshInstance->GetMesh() : nullptr;

	InstancedStaticMeshInstance* treeMeshInstance = treeInstancedStaticMeshComponent_->GetMeshInstance();
	InstancedStaticMesh* instancedTreeMesh = treeMeshInstance ? treeMeshInstance->GetMesh() : nullptr;

	if (!instancedGrassMesh || !instancedTreeMesh || instancedGrassMesh->GetInstanceCount() > 0 || instancedTreeMesh->GetInstanceCount() > 0)
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

	std::vector<Matrix> grassInstanceTransformationMatrices;
	grassInstanceTransformationMatrices.reserve(static_cast<size_t>(xCellCount) * static_cast<size_t>(yCellCount));

	std::vector<Matrix> treeInstanceTransformationMatrices;
	treeInstanceTransformationMatrices.reserve(static_cast<size_t>(xCellCount) * static_cast<size_t>(yCellCount) / 10);

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

			if (Vector3::Dot(raycastResult.hitNormal, Vector3(0.f, 0.f, 1.f)) < 0.75f)
			{
				continue;
			}

			float px = raycastResult.hitPosition.x * 0.2f;
			float py = raycastResult.hitPosition.y * 0.2f;

			float treeNoise = Perlin(px, py, 12345);
			float grassNoise = Perlin(px, py, 54321);

			bool spawnTree = false;

			if (treeNoise > 0.75f)
			{
				if (GoknarMath::GetRandom(0.f, 1.f) < 0.01f)
				{
					bool tooClose = false;
					for (const Matrix& treeTransform : treeInstanceTransformationMatrices)
					{
						Vector3 pos = Vector3(treeTransform[3], treeTransform[7], treeTransform[11]);
						Vector3 diff = pos - raycastResult.hitPosition;
						if (diff.x * diff.x + diff.y * diff.y + diff.z * diff.z < 4.f)
						{
							tooClose = true;
							break;
						}
					}

					if (!tooClose)
					{
						spawnTree = true;
					}
				}
			}
			else
			{
				if (GoknarMath::GetRandom(0.f, 1.f) < 0.001f)
				{
					bool tooClose = false;
					for (const Matrix& treeTransform : treeInstanceTransformationMatrices)
					{
						Vector3 pos = Vector3(treeTransform[3], treeTransform[7], treeTransform[11]);
						Vector3 diff = pos - raycastResult.hitPosition;
						if (diff.x * diff.x + diff.y * diff.y + diff.z * diff.z < 4.f)
						{
							tooClose = true;
							break;
						}
					}

					if (!tooClose)
					{
						spawnTree = true;
					}
				}
			}

			if (spawnTree)
			{
				const float randomRotationOnZ = GoknarMath::GetRandom(0.f, 360.f);

				treeInstanceTransformationMatrices.push_back(
					Matrix::GetTransformationMatrix(
						Quaternion::FromEulerDegrees(0.f, 0.f, randomRotationOnZ),
						raycastResult.hitPosition,
						Vector3(1.f)));

				TreeObject* treeObject = new TreeObject();
				treeObject->SetWorldPosition(raycastResult.hitPosition);
				treeObject->SetWorldRotation(Quaternion::FromEulerDegrees(0.f, 0.f, randomRotationOnZ));
			}
			else
			{
				bool spawnGrass = false;
				if (grassNoise > 0.45f)
				{
					// Dense grass inside Perlin zone
					spawnGrass = true;
				}
				else
				{
					// Mod: Grass spawned outside Perlin zone, but at a heavily reduced density (15%)
					if (GoknarMath::GetRandom(0.f, 1.f) < 0.15f)
					{
						spawnGrass = true;
					}
				}

				if (spawnGrass)
				{
					const float randomRotationOnZ = GoknarMath::GetRandom(kGrassMinimumRotationOnZ, kGrassMaximumRotationOnZ);
					const float randomUniformScale = GoknarMath::GetRandom(kGrassMinimumScale, kGrassMaximumScale);

					grassInstanceTransformationMatrices.push_back(
						Matrix::GetTransformationMatrix(
							Quaternion::FromEulerDegrees(0.f, 0.f, randomRotationOnZ),
							raycastResult.hitPosition,
							Vector3(randomUniformScale)));
				}
			}
		}
	}

	instancedGrassMesh->SetInstanceTransformations(grassInstanceTransformationMatrices);
	instancedGrassMesh->UpdateAllTransforms();

	instancedTreeMesh->SetInstanceTransformations(treeInstanceTransformationMatrices);
	instancedTreeMesh->UpdateAllTransforms();

	SceneParser::SaveScene(engine->GetApplication()->GetMainScene(), ContentDir + "Scenes/Scene_Terrain_Generated");
}