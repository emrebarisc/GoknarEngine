#include "pch.h"

#include "NavigationTreeComponent.h"

#include <unordered_map>

#include "Goknar/Helpers/ContentPathUtils.h"
#include "Goknar/Navigation/NavigationTreeSerializer.h"

namespace
{
	Vector3 TransformNavigationPoint(const Matrix& transform, const Vector3& point)
	{
		return Vector3(transform * Vector4(point, 1.f));
	}

	Area TransformNavigationArea(const Area& area, const Matrix* transform)
	{
		if (!transform)
		{
			return area;
		}

		Area transformedArea;
		transformedArea.point0 = TransformNavigationPoint(*transform, area.point0);
		transformedArea.point1 = TransformNavigationPoint(*transform, area.point1);
		transformedArea.point2 = TransformNavigationPoint(*transform, area.point2);
		transformedArea.point3 = TransformNavigationPoint(*transform, area.point3);
		return transformedArea;
	}

	void CopyNavigationTree(const NavigationTree& sourceTree, NavigationTree& targetTree, const Matrix* transform = nullptr)
	{
		targetTree.Clear();

		std::unordered_map<const NavigationNode*, NavigationNode*> copiedNodes;
		int nextNodeId = 0;

		for (const std::unique_ptr<NavigationNode>& sourceNode : sourceTree.GetNodes())
		{
			if (!sourceNode)
			{
				continue;
			}

			NavigationNode* copiedNode = targetTree.AddNode(TransformNavigationArea(sourceNode->area, transform));
			copiedNode->id = sourceNode->id;
			copiedNode->cost = sourceNode->cost;
			copiedNodes[sourceNode.get()] = copiedNode;
			nextNodeId = GoknarMath::Max(nextNodeId, sourceNode->id + 1);
		}

		for (const std::unique_ptr<NavigationNode>& sourceNode : sourceTree.GetNodes())
		{
			if (!sourceNode)
			{
				continue;
			}

			auto copiedNodeIterator = copiedNodes.find(sourceNode.get());
			if (copiedNodeIterator == copiedNodes.end())
			{
				continue;
			}

			NavigationNode* copiedNode = copiedNodeIterator->second;
			for (NavigationNode* sourceNeighbour : sourceNode->neighbours)
			{
				auto copiedNeighbourIterator = copiedNodes.find(sourceNeighbour);
				if (copiedNeighbourIterator != copiedNodes.end())
				{
					copiedNode->neighbours.push_back(copiedNeighbourIterator->second);
				}
			}
		}

		auto copiedRootIterator = copiedNodes.find(sourceTree.GetRoot());
		targetTree.SetRoot(copiedRootIterator != copiedNodes.end() ? copiedRootIterator->second : targetTree.GetRoot());
		targetTree.EnsureNextNodeId(nextNodeId);
	}
}

NavigationTreeComponent::NavigationTreeComponent(Component* parent) :
	Component(parent)
{
}

NavigationTreeComponent::~NavigationTreeComponent()
{
}

Component* NavigationTreeComponent::Clone() const
{
	NavigationTreeComponent* clonedComponent = new NavigationTreeComponent((Component*)nullptr);
	CopyValuesTo(clonedComponent);
	clonedComponent->navigationTreePath_ = navigationTreePath_;
	CopyNavigationTree(relativeNavigationTree_, clonedComponent->relativeNavigationTree_);
	clonedComponent->UpdateNavigationTree();

	return clonedComponent;
}

bool NavigationTreeComponent::SetNavigationTreePath(const std::string& navigationTreePath)
{
	navigationTreePath_ = ContentPathUtils::ToContentRelativePath(navigationTreePath);
	relativeNavigationTree_.Clear();
	navigationTree_.Clear();

	if (navigationTreePath_.empty())
	{
		return true;
	}

	const bool loadedNavigationTree = NavigationTreeSerializer::Deserialize(navigationTreePath_, relativeNavigationTree_);
	UpdateNavigationTree();
	return loadedNavigationTree;
}

void NavigationTreeComponent::SetRelativeNavigationTree(const NavigationTree& navigationTree)
{
	CopyNavigationTree(navigationTree, relativeNavigationTree_);
	UpdateNavigationTree();
}

void NavigationTreeComponent::UpdateNavigationTree()
{
	CopyNavigationTree(relativeNavigationTree_, navigationTree_, &GetComponentToWorldTransformationMatrix());
}

void NavigationTreeComponent::UpdateComponentToWorldTransformationMatrix()
{
	Component::UpdateComponentToWorldTransformationMatrix();
	UpdateNavigationTree();
}
