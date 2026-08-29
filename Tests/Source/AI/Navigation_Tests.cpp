#include "Navigation_Tests.h"

#include <cmath>
#include <cstdio>

#include "TestsCore.h"

#include "Goknar/Navigation/NavMeshBuilder.h"
#include "Goknar/Navigation/NavMeshFBXExporter.h"
#include "Goknar/Navigation/NavMeshGenerator.h"
#include "Goknar/Navigation/NavMeshSerializer.h"
#include "Goknar/Navigation/NavQuery.h"
#include "Goknar/Contents/Content.h"
#include "Goknar/IO/IOManager.h"
#include "Goknar/Model/StaticMesh.h"

namespace
{
	NavGeometryTriangle MakeTriangle(const Vector3& first, const Vector3& second, const Vector3& third, unsigned int sourceGeometryId = 0u, bool allowBlockingBridge = false)
	{
		NavGeometryTriangle triangle;
		triangle.vertices[0] = first;
		triangle.vertices[1] = second;
		triangle.vertices[2] = third;
		triangle.sourceGeometryId = sourceGeometryId;
		triangle.allowBlockingBridge = allowBlockingBridge;
		return triangle;
	}

	void AddQuad(std::vector<NavGeometryTriangle>& triangles, const Vector3& first, const Vector3& second, const Vector3& third, const Vector3& fourth, unsigned int sourceGeometryId = 0u, bool allowBlockingBridge = false)
	{
		triangles.push_back(MakeTriangle(first, second, fourth, sourceGeometryId, allowBlockingBridge));
		triangles.push_back(MakeTriangle(second, third, fourth, sourceGeometryId, allowBlockingBridge));
	}

	void AddHorizontalQuad(std::vector<NavGeometryTriangle>& triangles, float minX, float minY, float maxX, float maxY, float z, unsigned int sourceGeometryId = 0u)
	{
		AddQuad(
			triangles,
			Vector3(minX, minY, z),
			Vector3(maxX, minY, z),
			Vector3(maxX, maxY, z),
			Vector3(minX, maxY, z),
			sourceGeometryId);
	}

	void AddBox(std::vector<NavGeometryTriangle>& triangles, float minX, float minY, float maxX, float maxY, float minZ, float maxZ, unsigned int sourceGeometryId = 0u)
	{
		const Vector3 a(minX, minY, minZ);
		const Vector3 b(maxX, minY, minZ);
		const Vector3 c(maxX, maxY, minZ);
		const Vector3 d(minX, maxY, minZ);
		const Vector3 e(minX, minY, maxZ);
		const Vector3 f(maxX, minY, maxZ);
		const Vector3 g(maxX, maxY, maxZ);
		const Vector3 h(minX, maxY, maxZ);

		AddQuad(triangles, e, f, g, h, sourceGeometryId, true);
		triangles.push_back(MakeTriangle(a, d, b, sourceGeometryId, true));
		triangles.push_back(MakeTriangle(b, d, c, sourceGeometryId, true));
		AddQuad(triangles, a, b, f, e, sourceGeometryId, true);
		AddQuad(triangles, b, c, g, f, sourceGeometryId, true);
		AddQuad(triangles, c, d, h, g, sourceGeometryId, true);
		AddQuad(triangles, d, a, e, h, sourceGeometryId, true);
	}

	bool HasNearestPoly(const NavMesh& navMesh, const Vector3& point, const Vector3& extent)
	{
		const std::vector<NavTile>& tiles = navMesh.GetTiles();
		for (unsigned int tileIndex = 0; tileIndex < tiles.size(); ++tileIndex)
		{
			const NavTile& tile = tiles[tileIndex];
			for (unsigned int polygonIndex = 0; polygonIndex < tile.polygons.size(); ++polygonIndex)
			{
				const NavPolyRef polygonRef = navMesh.GetPolyRef(tileIndex, polygonIndex);
				if (polygonRef == InvalidNavPolyRef || !navMesh.IsPointInsidePoly(polygonRef, point))
				{
					continue;
				}

				Vector3 projectedPoint = Vector3::ZeroVector;
				if (navMesh.ProjectPointToPoly(polygonRef, point, projectedPoint) &&
					std::fabs(projectedPoint.x - point.x) <= extent.x &&
					std::fabs(projectedPoint.y - point.y) <= extent.y &&
					std::fabs(projectedPoint.z - point.z) <= extent.z)
				{
					return true;
				}
			}
		}

		return false;
	}

	bool HasSuccessfulPath(const NavMesh& navMesh, const Vector3& start, const Vector3& end)
	{
		NavQuery query(&navMesh);
		PathRequest request;
		request.start = start;
		request.end = end;
		request.queryExtent = Vector3(20.f, 20.f, 80.f);

		PathResult result;
		return query.FindPath(request, result) && result.status == NavPathStatus::Success;
	}

	bool AllPolygonsAreTriangles(const NavMesh& navMesh)
	{
		for (const NavTile& tile : navMesh.GetTiles())
		{
			for (const NavPolygon& polygon : tile.polygons)
			{
				if (polygon.vertexIndices.size() != 3u)
				{
					return false;
				}
			}
		}

		return true;
	}
}

void Navigation_Tests::Run()
{
	std::vector<Vector3> vertices =
	{
		Vector3(0.f, 0.f, 0.f),
		Vector3(100.f, 0.f, 0.f),
		Vector3(200.f, 0.f, 0.f),
		Vector3(0.f, 100.f, 0.f),
		Vector3(100.f, 100.f, 0.f),
		Vector3(200.f, 100.f, 0.f)
	};

	std::vector<NavBuildPolygon> polygons(2);
	polygons[0].vertexIndices = { 0, 1, 4, 3 };
	polygons[0].neighborPolygonIndices = { 1 };
	polygons[1].vertexIndices = { 1, 2, 5, 4 };
	polygons[1].neighborPolygonIndices = { 0 };

	NavMesh navMesh;
	TEST_ASSERT(NavMeshBuilder::BuildSingleTile(navMesh, vertices, polygons), "Navigation: manual single-tile navmesh built");

	NavQuery query(&navMesh);
	PathRequest request;
	request.start = Vector3(10.f, 50.f, 0.f);
	request.end = Vector3(190.f, 50.f, 0.f);

	PathResult result;
	TEST_ASSERT(query.FindPath(request, result), "Navigation: path query succeeds");
	TEST_ASSERT(result.status == NavPathStatus::Success, "Navigation: path status is Success");
	TEST_ASSERT(result.corridor.size() == 2, "Navigation: path corridor crosses two polygons");
	TEST_ASSERT(result.smoothedPath.size() >= 2, "Navigation: smoothed path has endpoints");

	const std::string testFilePath = "Navigation_Tests.gknav";
	TEST_ASSERT(NavMeshSerializer::Serialize(testFilePath, navMesh), "Navigation: navmesh serialized");

	NavMesh loadedNavMesh;
	TEST_ASSERT(NavMeshSerializer::Deserialize(testFilePath, loadedNavMesh), "Navigation: navmesh deserialized");

	NavQuery loadedQuery(&loadedNavMesh);
	PathResult loadedResult;
	TEST_ASSERT(loadedQuery.FindPath(request, loadedResult), "Navigation: loaded navmesh path query succeeds");
	TEST_ASSERT(loadedResult.status == NavPathStatus::Success, "Navigation: loaded navmesh path status is Success");

	std::vector<NavGeometryTriangle> generationTriangles(2);
	generationTriangles[0].vertices[0] = Vector3(0.f, 0.f, 0.f);
	generationTriangles[0].vertices[1] = Vector3(100.f, 0.f, 0.f);
	generationTriangles[0].vertices[2] = Vector3(0.f, 100.f, 0.f);
	generationTriangles[1].vertices[0] = Vector3(100.f, 0.f, 0.f);
	generationTriangles[1].vertices[1] = Vector3(100.f, 100.f, 0.f);
	generationTriangles[1].vertices[2] = Vector3(0.f, 100.f, 0.f);

	NavBuildSettings buildSettings;
	buildSettings.tileSize = 1000.f;
	NavMesh generatedNavMesh;
	TEST_ASSERT(NavMeshGenerator::GenerateFromTriangles(generationTriangles, buildSettings, generatedNavMesh), "Navigation: triangle generator succeeds");

	std::vector<NavGeometryTriangle> normalOrientedTriangles(2);
	normalOrientedTriangles[0] = MakeTriangle(Vector3(0.f, 0.f, 0.f), Vector3(0.f, 100.f, 0.f), Vector3(100.f, 0.f, 0.f));
	normalOrientedTriangles[1] = MakeTriangle(Vector3(100.f, 0.f, 0.f), Vector3(0.f, 100.f, 0.f), Vector3(100.f, 100.f, 0.f));
	for (NavGeometryTriangle& triangle : normalOrientedTriangles)
	{
		triangle.normal = Vector3::UpVector;
		triangle.hasNormal = true;
	}

	NavMesh normalOrientedNavMesh;
	TEST_ASSERT(NavMeshGenerator::GenerateFromTriangles(normalOrientedTriangles, buildSettings, normalOrientedNavMesh), "Navigation: authored normals orient reversed mesh winding");

	for (NavGeometryTriangle& triangle : normalOrientedTriangles)
	{
		triangle.normal = -Vector3::UpVector;
	}

	NavMesh downwardNormalNavMesh;
	TEST_ASSERT(!NavMeshGenerator::GenerateFromTriangles(normalOrientedTriangles, buildSettings, downwardNormalNavMesh), "Navigation: downward authored normals are not treated as walkable");

	std::vector<NavGeometryTriangle> defaultSettingsBoxFloorTriangles;
	AddBox(defaultSettingsBoxFloorTriangles, -20.f, -20.f, 20.f, 20.f, -0.125f, 0.125f, 900u);
	NavBuildSettings defaultPhysicsBuildSettings;
	defaultPhysicsBuildSettings.tileSize = 1000.f;
	NavMesh defaultSettingsBoxFloorNavMesh;
	TEST_ASSERT(NavMeshGenerator::GenerateFromTriangles(defaultSettingsBoxFloorTriangles, defaultPhysicsBuildSettings, defaultSettingsBoxFloorNavMesh), "Navigation: default settings generate navmesh from box collision floor");
	TEST_ASSERT(HasNearestPoly(defaultSettingsBoxFloorNavMesh, Vector3(0.f, 0.f, 0.125f), Vector3(0.5f, 0.5f, 0.5f)), "Navigation: primitive floor collision keeps its walkable top surface");

	NavQuery generatedQuery(&generatedNavMesh);
	PathResult generatedResult;
	PathRequest generatedRequest;
	generatedRequest.start = Vector3(5.f, 5.f, 0.f);
	generatedRequest.end = Vector3(95.f, 95.f, 0.f);
	TEST_ASSERT(generatedQuery.FindPath(generatedRequest, generatedResult), "Navigation: generated navmesh path query succeeds");

	NavBuildSettings validationSettings;
	validationSettings.tileSize = 1000.f;
	validationSettings.vertexMergeTolerance = 0.01f;
	validationSettings.minRegionArea = 0.25f;
	validationSettings.agentProfile.radius = 5.f;
	validationSettings.agentProfile.height = 80.f;
	validationSettings.agentProfile.maxStepHeight = 20.f;
	validationSettings.agentProfile.maxSlopeRadians = DEGREE_TO_RADIAN(45.f);

	std::vector<NavGeometryTriangle> flatPlaneTriangles;
	AddHorizontalQuad(flatPlaneTriangles, 0.f, 0.f, 200.f, 200.f, 0.f);
	NavMesh flatPlaneNavMesh;
	TEST_ASSERT(NavMeshGenerator::GenerateFromTriangles(flatPlaneTriangles, validationSettings, flatPlaneNavMesh), "Navigation: flat plane generates navmesh");
	TEST_ASSERT(HasSuccessfulPath(flatPlaneNavMesh, Vector3(10.f, 10.f, 0.f), Vector3(190.f, 190.f, 0.f)), "Navigation: flat plane remains fully walkable");

	NavBuildSettings meterSettings;
	meterSettings.tileSize = 1000.f;
	meterSettings.vertexMergeTolerance = 0.01f;
	meterSettings.minRegionArea = 0.01f;
	meterSettings.agentProfile.radius = 0.4f;
	meterSettings.agentProfile.height = 1.8f;
	meterSettings.agentProfile.maxStepHeight = 0.35f;
	meterSettings.agentProfile.maxSlopeRadians = DEGREE_TO_RADIAN(45.f);

	std::vector<NavGeometryTriangle> tallBoxObstacleTriangles;
	AddHorizontalQuad(tallBoxObstacleTriangles, -5.f, -5.f, 5.f, 5.f, 0.f, 1000u);
	AddBox(tallBoxObstacleTriangles, -1.f, -1.f, 1.f, 1.f, 0.f, 3.f, 1001u);
	NavMesh tallBoxObstacleNavMesh;
	TEST_ASSERT(NavMeshGenerator::GenerateFromTriangles(tallBoxObstacleTriangles, meterSettings, tallBoxObstacleNavMesh), "Navigation: tall box obstacle generates surrounding navmesh");
	TEST_ASSERT(!HasNearestPoly(tallBoxObstacleNavMesh, Vector3(0.f, 0.f, 0.f), Vector3(0.2f, 0.2f, 0.2f)), "Navigation: tall box side faces block their solid footprint");

	std::vector<NavGeometryTriangle> openBoundaryTriangles;
	AddHorizontalQuad(openBoundaryTriangles, -5.f, -5.f, 5.f, 5.f, 0.f, 1010u);
	AddBox(openBoundaryTriangles, -2.f, -2.f, -1.7f, 2.f, 0.f, 3.f, 1011u);
	AddBox(openBoundaryTriangles, -2.f, 1.7f, 2.f, 2.f, 0.f, 3.f, 1012u);
	AddBox(openBoundaryTriangles, -2.f, -2.f, 2.f, -1.7f, 0.f, 3.f, 1013u);
	NavMesh openBoundaryNavMesh;
	TEST_ASSERT(NavMeshGenerator::GenerateFromTriangles(openBoundaryTriangles, meterSettings, openBoundaryNavMesh), "Navigation: open boundary obstacles generate navmesh");
	TEST_ASSERT(HasNearestPoly(openBoundaryNavMesh, Vector3(0.f, 0.f, 0.f), Vector3(0.2f, 0.2f, 0.2f)), "Navigation: separate open boundary obstacles do not fill their convex hull");

	std::vector<NavGeometryTriangle> triangleMeshBoundaryTriangles;
	AddHorizontalQuad(triangleMeshBoundaryTriangles, -5.f, -5.f, 5.f, 5.f, 0.f, 1020u);
	AddQuad(
		triangleMeshBoundaryTriangles,
		Vector3(-5.f, -5.f, 0.f),
		Vector3(-5.f, 5.f, 0.f),
		Vector3(-5.f, 5.f, 3.f),
		Vector3(-5.f, -5.f, 3.f),
		1020u);
	AddQuad(
		triangleMeshBoundaryTriangles,
		Vector3(5.f, 5.f, 0.f),
		Vector3(5.f, -5.f, 0.f),
		Vector3(5.f, -5.f, 3.f),
		Vector3(5.f, 5.f, 3.f),
		1020u);
	AddQuad(
		triangleMeshBoundaryTriangles,
		Vector3(-5.f, 5.f, 0.f),
		Vector3(5.f, 5.f, 0.f),
		Vector3(5.f, 5.f, 3.f),
		Vector3(-5.f, 5.f, 3.f),
		1020u);
	AddQuad(
		triangleMeshBoundaryTriangles,
		Vector3(5.f, -5.f, 0.f),
		Vector3(-5.f, -5.f, 0.f),
		Vector3(-5.f, -5.f, 3.f),
		Vector3(5.f, -5.f, 3.f),
		1020u);
	NavMesh triangleMeshBoundaryNavMesh;
	TEST_ASSERT(NavMeshGenerator::GenerateFromTriangles(triangleMeshBoundaryTriangles, meterSettings, triangleMeshBoundaryNavMesh), "Navigation: closed triangle mesh boundary generates navmesh");
	TEST_ASSERT(HasNearestPoly(triangleMeshBoundaryNavMesh, Vector3(0.f, 0.f, 0.f), Vector3(0.2f, 0.2f, 0.2f)), "Navigation: triangle mesh boundary walls do not fill their interior hull");

	std::vector<NavGeometryTriangle> boxOnPlaneTriangles = flatPlaneTriangles;
	AddBox(boxOnPlaneTriangles, 75.f, 75.f, 125.f, 125.f, 0.f, 60.f);
	NavMesh boxOnPlaneNavMesh;
	NavMeshBuildDebugData boxDebugData;
	TEST_ASSERT(NavMeshGenerator::GenerateFromTriangles(boxOnPlaneTriangles, validationSettings, boxOnPlaneNavMesh, &boxDebugData), "Navigation: box on plane generates navmesh");
	TEST_ASSERT(AllPolygonsAreTriangles(boxOnPlaneNavMesh), "Navigation: generated subtraction polygons are triangulated");
	TEST_ASSERT(!HasNearestPoly(boxOnPlaneNavMesh, Vector3(100.f, 100.f, 0.f), Vector3(2.f, 2.f, 5.f)), "Navigation: box footprint is removed from the floor");
	TEST_ASSERT(HasNearestPoly(boxOnPlaneNavMesh, Vector3(25.f, 25.f, 0.f), Vector3(5.f, 5.f, 5.f)), "Navigation: floor outside box footprint remains walkable");
	TEST_ASSERT(!boxDebugData.blockedFootprintPolygons.empty(), "Navigation: obstacle footprint debug polygons are recorded");
	TEST_ASSERT(!boxDebugData.intersectionEdges.empty(), "Navigation: obstacle intersection debug edges are recorded");

	std::vector<NavGeometryTriangle> wallOnPlaneTriangles = flatPlaneTriangles;
	AddQuad(
		wallOnPlaneTriangles,
		Vector3(100.f, 0.f, 0.f),
		Vector3(100.f, 200.f, 0.f),
		Vector3(100.f, 200.f, 120.f),
		Vector3(100.f, 0.f, 120.f));
	NavMesh wallOnPlaneNavMesh;
	TEST_ASSERT(NavMeshGenerator::GenerateFromTriangles(wallOnPlaneTriangles, validationSettings, wallOnPlaneNavMesh), "Navigation: wall on plane generates navmesh");
	TEST_ASSERT(!HasNearestPoly(wallOnPlaneNavMesh, Vector3(100.f, 100.f, 0.f), Vector3(2.f, 2.f, 5.f)), "Navigation: wall footprint is removed from the floor");

	std::vector<NavGeometryTriangle> walkableRampTriangles;
	AddQuad(
		walkableRampTriangles,
		Vector3(0.f, 0.f, 0.f),
		Vector3(100.f, 0.f, 20.f),
		Vector3(100.f, 100.f, 20.f),
		Vector3(0.f, 100.f, 0.f));
	NavMesh walkableRampNavMesh;
	TEST_ASSERT(NavMeshGenerator::GenerateFromTriangles(walkableRampTriangles, validationSettings, walkableRampNavMesh), "Navigation: ramp under max slope is walkable");
	TEST_ASSERT(HasNearestPoly(walkableRampNavMesh, Vector3(50.f, 50.f, 10.f), Vector3(10.f, 10.f, 20.f)), "Navigation: ramp under max slope can be queried");

	std::vector<NavGeometryTriangle> steepRampOnPlaneTriangles = flatPlaneTriangles;
	AddQuad(
		steepRampOnPlaneTriangles,
		Vector3(50.f, 50.f, 0.f),
		Vector3(150.f, 50.f, 200.f),
		Vector3(150.f, 150.f, 200.f),
		Vector3(50.f, 150.f, 0.f));
	NavMesh steepRampOnPlaneNavMesh;
	TEST_ASSERT(NavMeshGenerator::GenerateFromTriangles(steepRampOnPlaneTriangles, validationSettings, steepRampOnPlaneNavMesh), "Navigation: steep ramp on plane generates surrounding navmesh");
	TEST_ASSERT(!HasNearestPoly(steepRampOnPlaneNavMesh, Vector3(100.f, 100.f, 0.f), Vector3(2.f, 2.f, 5.f)), "Navigation: ramp over max slope blocks its projected floor footprint");

	std::vector<NavGeometryTriangle> highPlatformTriangles;
	AddHorizontalQuad(highPlatformTriangles, 0.f, 0.f, 100.f, 100.f, 0.f);
	AddHorizontalQuad(highPlatformTriangles, 100.f, 0.f, 200.f, 100.f, 50.f);
	AddQuad(
		highPlatformTriangles,
		Vector3(100.f, 0.f, 0.f),
		Vector3(100.f, 100.f, 0.f),
		Vector3(100.f, 100.f, 50.f),
		Vector3(100.f, 0.f, 50.f));
	NavMesh highPlatformNavMesh;
	TEST_ASSERT(NavMeshGenerator::GenerateFromTriangles(highPlatformTriangles, validationSettings, highPlatformNavMesh), "Navigation: high platform generates navmesh islands");
	TEST_ASSERT(!HasSuccessfulPath(highPlatformNavMesh, Vector3(50.f, 50.f, 0.f), Vector3(150.f, 50.f, 50.f)), "Navigation: platform above max step height is not connected");

	std::vector<NavGeometryTriangle> lowPlatformTriangles;
	AddHorizontalQuad(lowPlatformTriangles, 0.f, 0.f, 100.f, 100.f, 0.f);
	AddHorizontalQuad(lowPlatformTriangles, 100.f, 0.f, 200.f, 100.f, 10.f);
	AddQuad(
		lowPlatformTriangles,
		Vector3(100.f, 0.f, 0.f),
		Vector3(100.f, 100.f, 0.f),
		Vector3(100.f, 100.f, 10.f),
		Vector3(100.f, 0.f, 10.f));
	NavMesh lowPlatformNavMesh;
	TEST_ASSERT(NavMeshGenerator::GenerateFromTriangles(lowPlatformTriangles, validationSettings, lowPlatformNavMesh), "Navigation: low platform generates navmesh");
	TEST_ASSERT(HasSuccessfulPath(lowPlatformNavMesh, Vector3(50.f, 50.f, 0.f), Vector3(150.f, 50.f, 10.f)), "Navigation: platform within max step height is connected");

	const std::string testFbxPath = "Navigation_Tests.fbx";
	TEST_ASSERT(NavMeshFBXExporter::Export(testFbxPath, generatedNavMesh), "Navigation: navmesh FBX export succeeds");
	Content* loadedContent = IOManager::LoadModel(testFbxPath);
	StaticMesh* loadedStaticMesh = dynamic_cast<StaticMesh*>(loadedContent);
	TEST_ASSERT(loadedStaticMesh != nullptr, "Navigation: exported navmesh FBX loads as StaticMesh");
	StaticMeshLOD* loadedStaticMeshLOD0 = loadedStaticMesh ? loadedStaticMesh->GetLOD(0) : nullptr;
	TEST_ASSERT(loadedStaticMeshLOD0 && !loadedStaticMeshLOD0->GetSubMeshes().empty(), "Navigation: exported navmesh FBX contains mesh data");
	delete loadedStaticMesh;

	const std::string testDebugFbxPath = "Navigation_Tests_Debug.fbx";
	TEST_ASSERT(NavMeshFBXExporter::ExportDebugBuild(testDebugFbxPath, boxOnPlaneNavMesh, boxDebugData), "Navigation: debug navmesh FBX export succeeds");

	std::remove(testFilePath.c_str());
	std::remove(testFbxPath.c_str());
	std::remove(testDebugFbxPath.c_str());

	GOKNAR_CORE_INFO("All Navigation test sequences completed.");
}
