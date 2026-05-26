#include "NavigationTree.h"

#include <algorithm>
#include <vector>

namespace
{
	constexpr int kNavigationBVHLeafNodeCount = 8;

	struct NavigationEdge
	{
		Vector3 start;
		Vector3 end;
		Vector3 center;
	};

	struct NavigationBounds
	{
		Vector3 min{ Vector3(MAX_FLOAT) };
		Vector3 max{ Vector3(-MAX_FLOAT) };
	};

	struct NavigationNodeEntry
	{
		NavigationNode* node{ nullptr };
		NavigationBounds bounds;
		Vector3 center{ Vector3::ZeroVector };
	};

	struct NavigationBVHNode
	{
		NavigationBounds bounds;
		int leftChildIndex{ -1 };
		int rightChildIndex{ -1 };
		int firstEntryIndex{ 0 };
		int entryCount{ 0 };

		bool IsLeaf() const
		{
			return entryCount > 0;
		}
	};

	const Vector3& GetAreaPoint(const Area& area, int pointIndex)
	{
		switch (pointIndex)
		{
		case 0: return area.point0;
		case 1: return area.point1;
		case 2: return area.point2;
		case 3: return area.point3;
		default: return area.point0;
		}
	}

	void ExtendBounds(NavigationBounds& bounds, const Vector3& point)
	{
		bounds.min = Vector3::Min(bounds.min, point);
		bounds.max = Vector3::Max(bounds.max, point);
	}

	void CombineBounds(NavigationBounds& bounds, const NavigationBounds& otherBounds)
	{
		bounds.min = Vector3::Min(bounds.min, otherBounds.min);
		bounds.max = Vector3::Max(bounds.max, otherBounds.max);
	}

	NavigationBounds GetAreaBounds(const Area& area)
	{
		NavigationBounds bounds;
		for (int pointIndex = 0; pointIndex < 4; ++pointIndex)
		{
			ExtendBounds(bounds, GetAreaPoint(area, pointIndex));
		}

		return bounds;
	}

	NavigationBounds GetExpandedBounds(const NavigationBounds& bounds, float horizontalExpansion, float verticalExpansion)
	{
		NavigationBounds expandedBounds = bounds;
		expandedBounds.min.x -= horizontalExpansion;
		expandedBounds.min.y -= horizontalExpansion;
		expandedBounds.min.z -= verticalExpansion;
		expandedBounds.max.x += horizontalExpansion;
		expandedBounds.max.y += horizontalExpansion;
		expandedBounds.max.z += verticalExpansion;
		return expandedBounds;
	}

	bool BoundsOverlap(const NavigationBounds& firstBounds, const NavigationBounds& secondBounds)
	{
		return firstBounds.min.x <= secondBounds.max.x &&
			secondBounds.min.x <= firstBounds.max.x &&
			firstBounds.min.y <= secondBounds.max.y &&
			secondBounds.min.y <= firstBounds.max.y &&
			firstBounds.min.z <= secondBounds.max.z &&
			secondBounds.min.z <= firstBounds.max.z;
	}

	Vector3 GetBoundsCenter(const NavigationBounds& bounds)
	{
		return (bounds.min + bounds.max) * 0.5f;
	}

	float GetBoundsAxisExtent(const NavigationBounds& bounds, int axis)
	{
		return bounds.max[axis] - bounds.min[axis];
	}

	int GetLargestBoundsAxis(const NavigationBounds& bounds)
	{
		const Vector3 extent = bounds.max - bounds.min;
		if (extent.y > extent.x && extent.y >= extent.z)
		{
			return 1;
		}

		return extent.z > extent.x && extent.z > extent.y ? 2 : 0;
	}

	class NavigationBVH
	{
	public:
		void Build(const NavigationTree& navigationTree)
		{
			entries_.clear();
			entryIndices_.clear();
			nodes_.clear();
			rootNodeIndex_ = -1;

			for (const std::unique_ptr<NavigationNode>& node : navigationTree.GetNodes())
			{
				if (!node)
				{
					continue;
				}

				NavigationNodeEntry entry;
				entry.node = node.get();
				entry.bounds = GetAreaBounds(node->area);
				entry.center = GetBoundsCenter(entry.bounds);
				entries_.push_back(entry);
			}

			entryIndices_.reserve(entries_.size());
			for (int entryIndex = 0; entryIndex < (int)entries_.size(); ++entryIndex)
			{
				entryIndices_.push_back(entryIndex);
			}

			if (!entries_.empty())
			{
				rootNodeIndex_ = BuildNode(0, (int)entryIndices_.size());
			}
		}

		template <typename CallbackType>
		void Query(const NavigationBounds& queryBounds, CallbackType&& callback) const
		{
			if (rootNodeIndex_ == -1)
			{
				return;
			}

			QueryNode(rootNodeIndex_, queryBounds, callback);
		}

	private:
		int BuildNode(int firstEntryIndex, int entryCount)
		{
			const int nodeIndex = (int)nodes_.size();
			nodes_.push_back(NavigationBVHNode{});

			NavigationBounds bounds;
			NavigationBounds centerBounds;
			for (int entryOffset = 0; entryOffset < entryCount; ++entryOffset)
			{
				const NavigationNodeEntry& entry = entries_[entryIndices_[firstEntryIndex + entryOffset]];
				CombineBounds(bounds, entry.bounds);
				ExtendBounds(centerBounds, entry.center);
			}

			nodes_[nodeIndex].bounds = bounds;

			const int splitAxis = GetLargestBoundsAxis(centerBounds);
			if (entryCount <= kNavigationBVHLeafNodeCount ||
				GetBoundsAxisExtent(centerBounds, splitAxis) <= SMALLER_EPSILON)
			{
				nodes_[nodeIndex].firstEntryIndex = firstEntryIndex;
				nodes_[nodeIndex].entryCount = entryCount;
				return nodeIndex;
			}

			std::sort(
				entryIndices_.begin() + firstEntryIndex,
				entryIndices_.begin() + firstEntryIndex + entryCount,
				[this, splitAxis](int firstEntryIndex, int secondEntryIndex)
				{
					return entries_[firstEntryIndex].center[splitAxis] < entries_[secondEntryIndex].center[splitAxis];
				});

			const int leftEntryCount = entryCount / 2;
			nodes_[nodeIndex].leftChildIndex = BuildNode(firstEntryIndex, leftEntryCount);
			nodes_[nodeIndex].rightChildIndex = BuildNode(firstEntryIndex + leftEntryCount, entryCount - leftEntryCount);
			return nodeIndex;
		}

		template <typename CallbackType>
		void QueryNode(int nodeIndex, const NavigationBounds& queryBounds, CallbackType& callback) const
		{
			const NavigationBVHNode& node = nodes_[nodeIndex];
			if (!BoundsOverlap(queryBounds, node.bounds))
			{
				return;
			}

			if (node.IsLeaf())
			{
				for (int entryOffset = 0; entryOffset < node.entryCount; ++entryOffset)
				{
					const NavigationNodeEntry& entry = entries_[entryIndices_[node.firstEntryIndex + entryOffset]];
					if (BoundsOverlap(queryBounds, entry.bounds))
					{
						callback(entry.node);
					}
				}

				return;
			}

			QueryNode(node.leftChildIndex, queryBounds, callback);
			QueryNode(node.rightChildIndex, queryBounds, callback);
		}

		std::vector<NavigationNodeEntry> entries_;
		std::vector<int> entryIndices_;
		std::vector<NavigationBVHNode> nodes_;
		int rootNodeIndex_{ -1 };
	};

	Vector2 ToHorizontalVector2(const Vector3& value)
	{
		return Vector2(value.x, value.y);
	}

	NavigationEdge GetAreaEdge(const Area& area, int edgeIndex)
	{
		const Vector3& start = GetAreaPoint(area, edgeIndex);
		const Vector3& end = GetAreaPoint(area, (edgeIndex + 1) % 4);
		return NavigationEdge{ start, end, (start + end) * 0.5f };
	}

	float Cross2D(const Vector2& first, const Vector2& second)
	{
		return first.x * second.y - first.y * second.x;
	}

	float PointSegmentDistance2D(const Vector2& point, const Vector2& segmentStart, const Vector2& segmentEnd)
	{
		const Vector2 segment = segmentEnd - segmentStart;
		const float segmentLengthSquared = segment.x * segment.x + segment.y * segment.y;
		if (segmentLengthSquared <= SMALLER_EPSILON)
		{
			return (point - segmentStart).Length();
		}

		const Vector2 pointToStart = point - segmentStart;
		const float t = GoknarMath::Clamp(Vector2::Dot(pointToStart, segment) / segmentLengthSquared, 0.f, 1.f);
		const Vector2 closestPoint = segmentStart + segment * t;
		return (point - closestPoint).Length();
	}

	bool SegmentsIntersect2D(const Vector2& firstStart, const Vector2& firstEnd, const Vector2& secondStart, const Vector2& secondEnd)
	{
		const Vector2 firstSegment = firstEnd - firstStart;
		const Vector2 secondSegment = secondEnd - secondStart;
		const float denominator = Cross2D(firstSegment, secondSegment);
		if (GoknarMath::Abs(denominator) <= SMALLER_EPSILON)
		{
			return false;
		}

		const Vector2 startDelta = secondStart - firstStart;
		const float t = Cross2D(startDelta, secondSegment) / denominator;
		const float u = Cross2D(startDelta, firstSegment) / denominator;
		return 0.f <= t && t <= 1.f && 0.f <= u && u <= 1.f;
	}

	float SegmentSegmentDistance2D(const NavigationEdge& firstEdge, const NavigationEdge& secondEdge)
	{
		const Vector2 firstStart = ToHorizontalVector2(firstEdge.start);
		const Vector2 firstEnd = ToHorizontalVector2(firstEdge.end);
		const Vector2 secondStart = ToHorizontalVector2(secondEdge.start);
		const Vector2 secondEnd = ToHorizontalVector2(secondEdge.end);

		if (SegmentsIntersect2D(firstStart, firstEnd, secondStart, secondEnd))
		{
			return 0.f;
		}

		return GoknarMath::Min(
			GoknarMath::Min(PointSegmentDistance2D(firstStart, secondStart, secondEnd), PointSegmentDistance2D(firstEnd, secondStart, secondEnd)),
			GoknarMath::Min(PointSegmentDistance2D(secondStart, firstStart, firstEnd), PointSegmentDistance2D(secondEnd, firstStart, firstEnd)));
	}

	float GetMinHorizontalEdgeDistanceAndStepHeight(const Area& firstArea, const Area& secondArea, float& outStepHeight)
	{
		float minDistance = MAX_FLOAT;
		outStepHeight = MAX_FLOAT;

		for (int firstEdgeIndex = 0; firstEdgeIndex < 4; ++firstEdgeIndex)
		{
			const NavigationEdge firstEdge = GetAreaEdge(firstArea, firstEdgeIndex);
			for (int secondEdgeIndex = 0; secondEdgeIndex < 4; ++secondEdgeIndex)
			{
				const NavigationEdge secondEdge = GetAreaEdge(secondArea, secondEdgeIndex);
				const float horizontalDistance = SegmentSegmentDistance2D(firstEdge, secondEdge);
				if (horizontalDistance < minDistance)
				{
					minDistance = horizontalDistance;
					outStepHeight = GoknarMath::Abs(firstEdge.center.z - secondEdge.center.z);
				}
			}
		}

		return minDistance;
	}

	void AddUniqueNeighbour(NavigationNode* node, NavigationNode* neighbour)
	{
		if (!node || !neighbour || node == neighbour)
		{
			return;
		}

		if (std::find(node->neighbours.begin(), node->neighbours.end(), neighbour) == node->neighbours.end())
		{
			node->neighbours.push_back(neighbour);
		}
	}
}

NavigationTree::NavigationTree()
{

}

NavigationTree::~NavigationTree()
{
	Clear();
}

NavigationNode* NavigationTree::AddNode(const Area& area)
{
	std::unique_ptr<NavigationNode> node = std::make_unique<NavigationNode>();
	node->id = nextNodeId_++;
	node->area = area;

	NavigationNode* nodeRaw = node.get();
	nodes_.push_back(std::move(node));

	if (!root)
	{
		root = nodeRaw;
	}

	return nodeRaw;
}

void NavigationTree::RemoveNode(NavigationNode* node)
{
	if (!node)
	{
		return;
	}

	for (std::unique_ptr<NavigationNode>& otherNode : nodes_)
	{
		if (!otherNode)
		{
			continue;
		}

		otherNode->neighbours.erase(
			std::remove(otherNode->neighbours.begin(), otherNode->neighbours.end(), node),
			otherNode->neighbours.end());
	}

	nodes_.erase(
		std::remove_if(
			nodes_.begin(),
			nodes_.end(),
			[node](const std::unique_ptr<NavigationNode>& candidate)
			{
				return candidate.get() == node;
			}),
		nodes_.end());

	if (root == node)
	{
		root = nodes_.empty() ? nullptr : nodes_.front().get();
	}
}

void NavigationTree::Clear()
{
	root = nullptr;
	nodes_.clear();
	nextNodeId_ = 0;
}

NavigationNode* NavigationTree::GetNodeById(int id) const
{
	for (const std::unique_ptr<NavigationNode>& node : nodes_)
	{
		if (node && node->id == id)
		{
			return node.get();
		}
	}

	return nullptr;
}

int NavigationTree::GetNodeIndex(const NavigationNode* node) const
{
	for (int nodeIndex = 0; nodeIndex < (int)nodes_.size(); ++nodeIndex)
	{
		if (nodes_[nodeIndex].get() == node)
		{
			return nodeIndex;
		}
	}

	return -1;
}

void NavigationTree::EnsureNextNodeId(int nextNodeId)
{
	nextNodeId_ = GoknarMath::Max(nextNodeId_, nextNodeId);
}

void NavigationTree::StitchNearbyNodes(
	NavigationTree& firstTree,
	NavigationTree& secondTree,
	float connectionDistance,
	float maxStepHeight)
{
	const float sanitizedConnectionDistance = GoknarMath::Max(connectionDistance, 0.f);
	const float sanitizedMaxStepHeight = GoknarMath::Max(maxStepHeight, 0.f);

	if (&firstTree == &secondTree)
	{
		return;
	}

	NavigationBVH secondTreeBVH;
	secondTreeBVH.Build(secondTree);

	for (const std::unique_ptr<NavigationNode>& firstNode : firstTree.GetNodes())
	{
		if (!firstNode)
		{
			continue;
		}

		const NavigationBounds queryBounds = GetExpandedBounds(
			GetAreaBounds(firstNode->area),
			sanitizedConnectionDistance,
			sanitizedMaxStepHeight);

		secondTreeBVH.Query(
			queryBounds,
			[firstNode = firstNode.get(), sanitizedConnectionDistance, sanitizedMaxStepHeight](NavigationNode* secondNode)
			{
				if (!secondNode)
				{
					return;
				}

				float stepHeight = 0.f;
				const float edgeDistance = GetMinHorizontalEdgeDistanceAndStepHeight(firstNode->area, secondNode->area, stepHeight);
				if (edgeDistance <= sanitizedConnectionDistance && stepHeight <= sanitizedMaxStepHeight)
				{
					AddUniqueNeighbour(firstNode, secondNode);
					AddUniqueNeighbour(secondNode, firstNode);
				}
			});
	}
}
