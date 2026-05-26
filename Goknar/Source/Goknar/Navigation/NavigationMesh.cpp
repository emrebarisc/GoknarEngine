#include "pch.h"

#include "NavigationMesh.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Goknar/Components/Component.h"
#include "Goknar/Engine.h"
#include "Goknar/ObjectBase.h"
#include "Goknar/Scene.h"
#include "Goknar/Navigation/NavigationTreeComponent.h"

namespace
{
	constexpr float kNavigationTreeStitchConnectionDistance = 0.1f;

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

	void AppendNavigationTreeNodes(
		const NavigationTree& sourceTree,
		NavigationTree& targetTree,
		std::unordered_map<const NavigationNode*, NavigationNode*>& copiedNodes)
	{
		const bool targetHadRoot = targetTree.GetRoot() != nullptr;

		for (const std::unique_ptr<NavigationNode>& sourceNode : sourceTree.GetNodes())
		{
			if (!sourceNode)
			{
				continue;
			}

			NavigationNode* copiedNode = targetTree.AddNode(sourceNode->area);
			copiedNode->cost = sourceNode->cost;
			copiedNodes[sourceNode.get()] = copiedNode;
		}

		if (targetHadRoot)
		{
			return;
		}

		auto copiedRootIterator = copiedNodes.find(sourceTree.GetRoot());
		if (copiedRootIterator != copiedNodes.end())
		{
			targetTree.SetRoot(copiedRootIterator->second);
		}
	}

	void CopyNavigationTreeNeighbours(
		const NavigationTree& sourceTree,
		const std::unordered_map<const NavigationNode*, NavigationNode*>& copiedNodes)
	{
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
					AddUniqueNeighbour(copiedNode, copiedNeighbourIterator->second);
				}
			}
		}
	}

	void CollectSceneNavigationTrees(
		Scene* scene,
		std::vector<NavigationTreeComponent*>& navigationTreeComponents,
		std::vector<ObjectBase*>& navigationTreeObjects)
	{
		if (!scene)
		{
			return;
		}

		for (ObjectBase* object : scene->GetObjects())
		{
			if (!object)
			{
				continue;
			}

			bool hasNavigationTreeComponent = false;
			for (Component* component : object->GetComponents())
			{
				NavigationTreeComponent* navigationTreeComponent = dynamic_cast<NavigationTreeComponent*>(component);
				if (navigationTreeComponent)
				{
					navigationTreeComponents.push_back(navigationTreeComponent);
					hasNavigationTreeComponent = true;
				}
			}

			if (hasNavigationTreeComponent)
			{
				navigationTreeObjects.push_back(object);
			}
		}
	}

	void StitchNavigationTreeComponents(
		const std::vector<NavigationTreeComponent*>& navigationTreeComponents,
		const NavMeshSettings& settings)
	{
		for (NavigationTreeComponent* navigationTreeComponent : navigationTreeComponents)
		{
			if (navigationTreeComponent)
			{
				navigationTreeComponent->UpdateNavigationTree();
			}
		}

		for (size_t firstComponentIndex = 0; firstComponentIndex < navigationTreeComponents.size(); ++firstComponentIndex)
		{
			NavigationTreeComponent* firstComponent = navigationTreeComponents[firstComponentIndex];
			if (!firstComponent)
			{
				continue;
			}

			for (size_t secondComponentIndex = firstComponentIndex + 1; secondComponentIndex < navigationTreeComponents.size(); ++secondComponentIndex)
			{
				NavigationTreeComponent* secondComponent = navigationTreeComponents[secondComponentIndex];
				if (!secondComponent)
				{
					continue;
				}

				NavigationTree::StitchNearbyNodes(
					firstComponent->GetNavigationTree(),
					secondComponent->GetNavigationTree(),
					kNavigationTreeStitchConnectionDistance,
					settings.maxStepSize);
			}
		}
	}

	void CopyNavigationTreeComponentsToMesh(
		const std::vector<NavigationTreeComponent*>& navigationTreeComponents,
		NavigationTree& navigationTree)
	{
		std::unordered_map<const NavigationNode*, NavigationNode*> copiedNodes;
		for (NavigationTreeComponent* navigationTreeComponent : navigationTreeComponents)
		{
			if (navigationTreeComponent)
			{
				AppendNavigationTreeNodes(navigationTreeComponent->GetNavigationTree(), navigationTree, copiedNodes);
			}
		}

		for (NavigationTreeComponent* navigationTreeComponent : navigationTreeComponents)
		{
			if (navigationTreeComponent)
			{
				CopyNavigationTreeNeighbours(navigationTreeComponent->GetNavigationTree(), copiedNodes);
			}
		}
	}

	void RemoveNavigationTreeObjects(Scene* scene, const std::vector<ObjectBase*>& navigationTreeObjects)
	{
		if (!scene || navigationTreeObjects.empty())
		{
			return;
		}

		std::unordered_set<ObjectBase*> removedObjects;
		for (ObjectBase* navigationTreeObject : navigationTreeObjects)
		{
			if (!navigationTreeObject || removedObjects.find(navigationTreeObject) != removedObjects.end())
			{
				continue;
			}

			removedObjects.insert(navigationTreeObject);
			scene->RemoveObject(navigationTreeObject);
			navigationTreeObject->Destroy();
		}

		engine->FlushPendingDestroy();
	}
}

void NavigationMesh::Clear()
{
	navigationTree_.Clear();
}

void NavigationMesh::BuildFromScene(Scene* scene, const NavMeshSettings& settings, bool removeNavigationTreeObjects)
{
	navigationTree_.Clear();
	if (!scene)
	{
		return;
	}

	std::vector<NavigationTreeComponent*> navigationTreeComponents;
	std::vector<ObjectBase*> navigationTreeObjects;
	CollectSceneNavigationTrees(scene, navigationTreeComponents, navigationTreeObjects);

	StitchNavigationTreeComponents(navigationTreeComponents, settings);
	CopyNavigationTreeComponentsToMesh(navigationTreeComponents, navigationTree_);

	if (removeNavigationTreeObjects)
	{
		RemoveNavigationTreeObjects(scene, navigationTreeObjects);
	}
}
