#ifndef __NAVIGATION_TYPES__
#define __NAVIGATION_TYPES__

#include "Goknar/Core.h"
#include "Goknar/Geometry/Box.h"

#include <vector>

#include "Goknar/Math/GoknarMath.h"

struct GOKNAR_API NavMeshSettings
{
	float maxSlopeAngle{ 30.f };
	float maxStepSize{ 0.35f };
};

struct GOKNAR_API NavMeshVertex
{
	Vector3 position;
};

struct GOKNAR_API NavMeshTriangle
{
	unsigned int index0;
	unsigned int index1;
	unsigned int index2;
};

struct GOKNAR_API NavMeshIntersectionLine
{
	Vector3 vertex0;
	Vector3 vertex1;
};

struct GOKNAR_API NavMeshIntersectionTriangle
{
	Vector3 vertex0;
	Vector3 vertex1;
	Vector3 vertex2;
};

struct GOKNAR_API NavMeshUnit
{
	NavMeshUnit() = default;
	~NavMeshUnit() = default;

	void AddVertex(const NavMeshVertex& navMeshVertex, int index = -1)
	{
		if (0 <= index)
		{
			vertices[index] = navMeshVertex;
		}
		else
		{
			vertices.push_back(navMeshVertex);
		}

		aabb.ExtendWRTPoint(navMeshVertex.position);
	}

	void AddTriangle(const NavMeshTriangle& navMeshTriangle)
	{
		triangles.push_back(navMeshTriangle);
	}

	std::vector<NavMeshVertex> vertices;
	std::vector<NavMeshTriangle> triangles;

	Box aabb;
};

#endif
