#include "pch.h"

#include "PathFinder.h"

#include "NavigationTree.h"

#include <algorithm>
#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
	constexpr float PortalPointEpsilonSquared = 0.0001f;
	constexpr float PathSmoothingFactor = 0.5f;
	constexpr float LookAheadSmoothingFactor = 0.35f;

	struct OpenNode
	{
		NavigationNode* node{ nullptr };
		float priority{ 0.f };
	};

	struct OpenNodeComparator
	{
		bool operator()(const OpenNode& first, const OpenNode& second) const
		{
			return first.priority > second.priority;
		}
	};

	struct Portal
	{
		Vector3 left;
		Vector3 right;
	};

	float Cross2D(const Vector3& first, const Vector3& second)
	{
		return first.x * second.y - first.y * second.x;
	}

	float TriangleArea2D(const Vector3& first, const Vector3& second, const Vector3& third)
	{
		return Cross2D(second - first, third - first);
	}

	bool AreSameHorizontalPoint(const Vector3& first, const Vector3& second)
	{
		const float deltaX = first.x - second.x;
		const float deltaY = first.y - second.y;
		return deltaX * deltaX + deltaY * deltaY <= PortalPointEpsilonSquared;
	}

	bool AreSamePoint(const Vector3& first, const Vector3& second)
	{
		return (first - second).SquareLength() <= PortalPointEpsilonSquared;
	}

	float GetDistance(const Vector3& first, const Vector3& second)
	{
		return (first - second).Length();
	}

	float GetTriangleSign(const Vector3& first, const Vector3& second, const Vector3& third)
	{
		return (first.x - third.x) * (second.y - third.y) - (second.x - third.x) * (first.y - third.y);
	}

	bool IsPointInTriangle2D(const Vector3& point, const Vector3& first, const Vector3& second, const Vector3& third)
	{
		const float sign0 = GetTriangleSign(point, first, second);
		const float sign1 = GetTriangleSign(point, second, third);
		const float sign2 = GetTriangleSign(point, third, first);

		const bool hasNegative = sign0 < 0.f || sign1 < 0.f || sign2 < 0.f;
		const bool hasPositive = 0.f < sign0 || 0.f < sign1 || 0.f < sign2;
		return !(hasNegative && hasPositive);
	}

	bool AreaContainsHorizontalPoint(const Area& area, const Vector3& position)
	{
		return IsPointInTriangle2D(position, area.point0, area.point1, area.point2) ||
			IsPointInTriangle2D(position, area.point0, area.point2, area.point3);
	}

	bool TryGetTriangleHeightAtHorizontalPoint(
		const Vector3& point,
		const Vector3& trianglePoint0,
		const Vector3& trianglePoint1,
		const Vector3& trianglePoint2,
		float& height)
	{
		const float denominator =
			(trianglePoint1.y - trianglePoint2.y) * (trianglePoint0.x - trianglePoint2.x) +
			(trianglePoint2.x - trianglePoint1.x) * (trianglePoint0.y - trianglePoint2.y);

		if (GoknarMath::Abs(denominator) <= SMALLER_EPSILON)
		{
			return false;
		}

		const float weight0 = ((trianglePoint1.y - trianglePoint2.y) * (point.x - trianglePoint2.x) +
			(trianglePoint2.x - trianglePoint1.x) * (point.y - trianglePoint2.y)) / denominator;
		const float weight1 = ((trianglePoint2.y - trianglePoint0.y) * (point.x - trianglePoint2.x) +
			(trianglePoint0.x - trianglePoint2.x) * (point.y - trianglePoint2.y)) / denominator;
		const float weight2 = 1.f - weight0 - weight1;

		height = weight0 * trianglePoint0.z + weight1 * trianglePoint1.z + weight2 * trianglePoint2.z;
		return true;
	}

	float GetAreaHeightAtHorizontalPoint(const Area& area, const Vector3& point, float fallbackHeight)
	{
		float height = fallbackHeight;
		if (IsPointInTriangle2D(point, area.point0, area.point1, area.point2) &&
			TryGetTriangleHeightAtHorizontalPoint(point, area.point0, area.point1, area.point2, height))
		{
			return height;
		}

		if (IsPointInTriangle2D(point, area.point0, area.point2, area.point3) &&
			TryGetTriangleHeightAtHorizontalPoint(point, area.point0, area.point2, area.point3, height))
		{
			return height;
		}

		return fallbackHeight;
	}

	Vector3 GetPointOnArea(const Area& area, const Vector3& horizontalPoint, float fallbackHeight)
	{
		return Vector3(horizontalPoint.x, horizontalPoint.y, GetAreaHeightAtHorizontalPoint(area, horizontalPoint, fallbackHeight));
	}

	Vector3 GetClosestPointOnSegment2D(const Vector3& point, const Vector3& segmentStart, const Vector3& segmentEnd)
	{
		const float segmentX = segmentEnd.x - segmentStart.x;
		const float segmentY = segmentEnd.y - segmentStart.y;
		const float segmentLengthSquared = segmentX * segmentX + segmentY * segmentY;

		if (segmentLengthSquared <= SMALLER_EPSILON)
		{
			return segmentStart;
		}

		const float projection = ((point.x - segmentStart.x) * segmentX + (point.y - segmentStart.y) * segmentY) / segmentLengthSquared;
		const float clampedProjection = GoknarMath::Max(0.f, GoknarMath::Min(1.f, projection));
		return Vector3(
			segmentStart.x + segmentX * clampedProjection,
			segmentStart.y + segmentY * clampedProjection,
			segmentStart.z + (segmentEnd.z - segmentStart.z) * clampedProjection);
	}

	Vector3 ClampPointToArea(const Area& area, const Vector3& position)
	{
		if (AreaContainsHorizontalPoint(area, position))
		{
			return GetPointOnArea(area, position, position.z);
		}

		const Vector3 areaPoints[4] = { area.point0, area.point1, area.point2, area.point3 };
		Vector3 closestPoint = area.point0;
		float closestDistanceSquared = MAX_FLOAT;

		for (int pointIndex = 0; pointIndex < 4; ++pointIndex)
		{
			const Vector3 edgeStart = areaPoints[pointIndex];
			const Vector3 edgeEnd = areaPoints[(pointIndex + 1) % 4];
			const Vector3 edgePoint = GetClosestPointOnSegment2D(position, edgeStart, edgeEnd);

			const float deltaX = position.x - edgePoint.x;
			const float deltaY = position.y - edgePoint.y;
			const float distanceSquared = deltaX * deltaX + deltaY * deltaY;

			if (distanceSquared < closestDistanceSquared)
			{
				closestDistanceSquared = distanceSquared;
				closestPoint = edgePoint;
			}
		}

		return GetPointOnArea(area, closestPoint, closestPoint.z);
	}

	Vector3 LerpVector3(const Vector3& first, const Vector3& second, float factor)
	{
		return first + (second - first) * factor;
	}

	void AddPathPoint(std::vector<NavigationPath>& path, const Vector3& position)
	{
		if (!path.empty() && (path.back().position - position).SquareLength() <= SMALLER_EPSILON)
		{
			return;
		}

		NavigationPath pathPoint;
		pathPoint.position = position;
		path.push_back(pathPoint);
	}

	NavigationNode* FindNodeAtPosition(const NavigationTree* navigationTree, const Vector3& position)
	{
		if (!navigationTree)
		{
			return nullptr;
		}

		NavigationNode* closestContainingNode = nullptr;
		float closestHeightDistance = MAX_FLOAT;

		NavigationNode* closestNode = nullptr;
		float closestDistanceSquared = MAX_FLOAT;

		for (const std::unique_ptr<NavigationNode>& node : navigationTree->GetNodes())
		{
			if (!node)
			{
				continue;
			}

			const Vector3 center = node->area.GetCenter();
			const float distanceSquared = (center - position).SquareLength();
			if (distanceSquared < closestDistanceSquared)
			{
				closestDistanceSquared = distanceSquared;
				closestNode = node.get();
			}

			if (!AreaContainsHorizontalPoint(node->area, position))
			{
				continue;
			}

			const float heightDistance = GoknarMath::Abs(center.z - position.z);
			if (heightDistance < closestHeightDistance)
			{
				closestHeightDistance = heightDistance;
				closestContainingNode = node.get();
			}
		}

		return closestContainingNode ? closestContainingNode : closestNode;
	}

	bool TryGetSharedPortal(const Area& firstArea, const Area& secondArea, Portal& portal)
	{
		const Vector3 firstPoints[4] = { firstArea.point0, firstArea.point1, firstArea.point2, firstArea.point3 };
		const Vector3 secondPoints[4] = { secondArea.point0, secondArea.point1, secondArea.point2, secondArea.point3 };

		std::vector<Vector3> sharedPoints;
		for (const Vector3& firstPoint : firstPoints)
		{
			for (const Vector3& secondPoint : secondPoints)
			{
				if (!AreSameHorizontalPoint(firstPoint, secondPoint))
				{
					continue;
				}

				bool alreadyAdded = false;
				for (const Vector3& sharedPoint : sharedPoints)
				{
					if (AreSameHorizontalPoint(sharedPoint, firstPoint))
					{
						alreadyAdded = true;
						break;
					}
				}

				if (!alreadyAdded)
				{
					sharedPoints.push_back(firstPoint);
				}
			}
		}

		if (sharedPoints.size() < 2)
		{
			return false;
		}

		portal.left = sharedPoints[0];
		portal.right = sharedPoints[1];

		const Vector3 direction = secondArea.GetCenter() - firstArea.GetCenter();
		const float leftCross = Cross2D(direction, portal.left - firstArea.GetCenter());
		const float rightCross = Cross2D(direction, portal.right - firstArea.GetCenter());

		if (leftCross < rightCross)
		{
			std::swap(portal.left, portal.right);
		}

		return true;
	}

	bool TryBuildFunnelPath(const Vector3& start, const Vector3& goal, const std::vector<NavigationNode*>& nodePath, std::vector<NavigationPath>& resultPath)
	{
		resultPath.clear();

		if (nodePath.empty())
		{
			return false;
		}

		std::vector<Portal> portals;
		portals.reserve(nodePath.size() + 1);
		portals.push_back(Portal{ start, start });

		for (int nodeIndex = 0; nodeIndex < (int)nodePath.size() - 1; ++nodeIndex)
		{
			NavigationNode* firstNode = nodePath[nodeIndex];
			NavigationNode* secondNode = nodePath[nodeIndex + 1];
			if (!firstNode || !secondNode)
			{
				return false;
			}

			Portal portal;
			if (!TryGetSharedPortal(firstNode->area, secondNode->area, portal))
			{
				return false;
			}

			portals.push_back(portal);
		}

		portals.push_back(Portal{ goal, goal });

		Vector3 portalApex = portals[0].left;
		Vector3 portalLeft = portals[0].left;
		Vector3 portalRight = portals[0].right;

		int apexIndex = 0;
		int leftIndex = 0;
		int rightIndex = 0;

		AddPathPoint(resultPath, portalApex);

		for (int portalIndex = 1; portalIndex < (int)portals.size(); ++portalIndex)
		{
			const Vector3 left = portals[portalIndex].left;
			const Vector3 right = portals[portalIndex].right;

			if (TriangleArea2D(portalApex, portalRight, right) <= 0.f)
			{
				if (AreSamePoint(portalApex, portalRight) || TriangleArea2D(portalApex, portalLeft, right) > 0.f)
				{
					portalRight = right;
					rightIndex = portalIndex;
				}
				else
				{
					AddPathPoint(resultPath, portalLeft);
					portalApex = portalLeft;
					apexIndex = leftIndex;
					portalLeft = portalApex;
					portalRight = portalApex;
					leftIndex = apexIndex;
					rightIndex = apexIndex;
					portalIndex = apexIndex;
					continue;
				}
			}

			if (TriangleArea2D(portalApex, portalLeft, left) >= 0.f)
			{
				if (AreSamePoint(portalApex, portalLeft) || TriangleArea2D(portalApex, portalRight, left) < 0.f)
				{
					portalLeft = left;
					leftIndex = portalIndex;
				}
				else
				{
					AddPathPoint(resultPath, portalRight);
					portalApex = portalRight;
					apexIndex = rightIndex;
					portalLeft = portalApex;
					portalRight = portalApex;
					leftIndex = apexIndex;
					rightIndex = apexIndex;
					portalIndex = apexIndex;
					continue;
				}
			}
		}

		AddPathPoint(resultPath, goal);
		return resultPath.size() > 1;
	}

	Vector3 GetNodePathAnchor(const std::vector<NavigationNode*>& nodePath, int nodeIndex, const Vector3& start, const Vector3& goal)
	{
		if (nodeIndex <= 0)
		{
			return start;
		}

		if (nodeIndex >= (int)nodePath.size() - 1)
		{
			return goal;
		}

		return nodePath[nodeIndex]->area.GetCenter();
	}

	Vector3 GetLookAheadAnchor(const std::vector<NavigationNode*>& nodePath, int nodeIndex, const Vector3& start, const Vector3& goal)
	{
		const int nextIndex = GoknarMath::Min(nodeIndex + 1, (int)nodePath.size() - 1);
		const Vector3 nextAnchor = GetNodePathAnchor(nodePath, nextIndex, start, goal);

		const int followingIndex = GoknarMath::Min(nodeIndex + 2, (int)nodePath.size() - 1);
		if (followingIndex == nextIndex)
		{
			return nextAnchor;
		}

		const Vector3 followingAnchor = GetNodePathAnchor(nodePath, followingIndex, start, goal);
		return LerpVector3(nextAnchor, followingAnchor, LookAheadSmoothingFactor);
	}

	void BuildFallbackSmoothedPath(const Vector3& start, const Vector3& goal, const std::vector<NavigationNode*>& nodePath, std::vector<NavigationPath>& resultPath)
	{
		resultPath.clear();
		AddPathPoint(resultPath, start);

		for (int nodeIndex = 0; nodeIndex < (int)nodePath.size() - 1; ++nodeIndex)
		{
			NavigationNode* node = nodePath[nodeIndex];
			if (!node)
			{
				continue;
			}

			const Vector3 anchor = GetNodePathAnchor(nodePath, nodeIndex, start, goal);
			const Vector3 lookAheadAnchor = GetLookAheadAnchor(nodePath, nodeIndex, start, goal);
			const Vector3 smoothedPoint = ClampPointToArea(node->area, LerpVector3(anchor, lookAheadAnchor, PathSmoothingFactor));
			AddPathPoint(resultPath, smoothedPoint);
		}

		AddPathPoint(resultPath, goal);
	}
}

bool PathFinder::FindPath(const Vector3& start, const Vector3& goal, std::vector<NavigationPath>& path)
{
	path.clear();

	if (!navigationTree_ || !navigationTree_->GetRoot())
	{
		return false;
	}

	NavigationNode* startNode = FindNodeAtPosition(navigationTree_, start);
	NavigationNode* goalNode = FindNodeAtPosition(navigationTree_, goal);
	if (!startNode || !goalNode)
	{
		return false;
	}

	if (startNode == goalNode)
	{
		AddPathPoint(path, start);
		AddPathPoint(path, goal);
		return !path.empty();
	}

	std::priority_queue<OpenNode, std::vector<OpenNode>, OpenNodeComparator> openNodes;
	std::unordered_map<NavigationNode*, NavigationNode*> cameFrom;
	std::unordered_map<NavigationNode*, float> gScores;
	std::unordered_set<NavigationNode*> closedNodes;

	cameFrom[startNode] = nullptr;
	gScores[startNode] = 0.f;
	openNodes.push(OpenNode{ startNode, GetDistance(startNode->area.GetCenter(), goalNode->area.GetCenter()) });

	while (!openNodes.empty())
	{
		NavigationNode* currentNode = openNodes.top().node;
		openNodes.pop();

		if (!currentNode || closedNodes.find(currentNode) != closedNodes.end())
		{
			continue;
		}

		if (currentNode == goalNode)
		{
			std::vector<NavigationNode*> nodePath;
			for (NavigationNode* node = goalNode; node != nullptr; )
			{
				nodePath.push_back(node);

				auto cameFromIterator = cameFrom.find(node);
				if (cameFromIterator == cameFrom.end())
				{
					path.clear();
					return false;
				}

				node = cameFromIterator->second;
			}

			std::reverse(nodePath.begin(), nodePath.end());
			GetSmoothPath(start, goal, nodePath, path);
			return path.size() > 1;
		}

		closedNodes.insert(currentNode);

		const float currentGScore = gScores[currentNode];
		const Vector3 currentCenter = currentNode->area.GetCenter();

		for (NavigationNode* neighbour : currentNode->neighbours)
		{
			if (!neighbour || closedNodes.find(neighbour) != closedNodes.end())
			{
				continue;
			}

			const float distance = GetDistance(currentCenter, neighbour->area.GetCenter());
			const float traversalCost = GoknarMath::Max(0.f, neighbour->cost);
			const float tentativeGScore = currentGScore + distance * traversalCost;

			auto neighbourGScoreIterator = gScores.find(neighbour);
			if (neighbourGScoreIterator != gScores.end() && neighbourGScoreIterator->second <= tentativeGScore)
			{
				continue;
			}

			cameFrom[neighbour] = currentNode;
			gScores[neighbour] = tentativeGScore;

			const float heuristic = GetDistance(neighbour->area.GetCenter(), goalNode->area.GetCenter());
			openNodes.push(OpenNode{ neighbour, tentativeGScore + heuristic });
		}
	}

	path.clear();
	return false;
}

void PathFinder::GetSmoothPath(const Vector3& start, const Vector3& goal, const std::vector<NavigationNode*>& nodePath, std::vector<NavigationPath>& resultPath)
{
	if (TryBuildFunnelPath(start, goal, nodePath, resultPath))
	{
		return;
	}

	BuildFallbackSmoothedPath(start, goal, nodePath, resultPath);
}
