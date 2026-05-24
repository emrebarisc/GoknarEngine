#include "NavigationTreeSerializer.h"

#include <unordered_map>

#include "tinyxml2.h"

#include "Goknar/Data/DataEncryption.h"
#include "Goknar/Helpers/ContentPathUtils.h"
#include "Goknar/Navigation/NavigationTree.h"

namespace
{
	constexpr const char* kNavigationTreeFileType = "NavigationTree";

	void WriteVector3(tinyxml2::XMLElement* element, const Vector3& value)
	{
		element->SetAttribute("X", value.x);
		element->SetAttribute("Y", value.y);
		element->SetAttribute("Z", value.z);
	}

	Vector3 ReadVector3(const tinyxml2::XMLElement* element)
	{
		Vector3 value = Vector3::ZeroVector;
		if (!element)
		{
			return value;
		}

		element->QueryFloatAttribute("X", &value.x);
		element->QueryFloatAttribute("Y", &value.y);
		element->QueryFloatAttribute("Z", &value.z);
		return value;
	}

	void WriteArea(tinyxml2::XMLDocument& document, tinyxml2::XMLElement* nodeElement, const Area& area)
	{
		tinyxml2::XMLElement* areaElement = document.NewElement("Area");
		const Vector3 points[4] = { area.point0, area.point1, area.point2, area.point3 };
		for (int pointIndex = 0; pointIndex < 4; ++pointIndex)
		{
			tinyxml2::XMLElement* pointElement = document.NewElement("Point");
			pointElement->SetAttribute("Index", pointIndex);
			WriteVector3(pointElement, points[pointIndex]);
			areaElement->InsertEndChild(pointElement);
		}

		nodeElement->InsertEndChild(areaElement);
	}

	Area ReadArea(const tinyxml2::XMLElement* nodeElement)
	{
		Area area;
		const tinyxml2::XMLElement* areaElement = nodeElement ? nodeElement->FirstChildElement("Area") : nullptr;
		for (const tinyxml2::XMLElement* pointElement = areaElement ? areaElement->FirstChildElement("Point") : nullptr;
			pointElement;
			pointElement = pointElement->NextSiblingElement("Point"))
		{
			int pointIndex = 0;
			pointElement->QueryIntAttribute("Index", &pointIndex);
			switch (pointIndex)
			{
			case 0:
				area.point0 = ReadVector3(pointElement);
				break;
			case 1:
				area.point1 = ReadVector3(pointElement);
				break;
			case 2:
				area.point2 = ReadVector3(pointElement);
				break;
			case 3:
				area.point3 = ReadVector3(pointElement);
				break;
			default:
				break;
			}
		}

		return area;
	}
}

bool NavigationTreeSerializer::Serialize(const std::string& filepath, const NavigationTree& navigationTree)
{
	const std::string relativeFilePath = ContentPathUtils::ToContentRelativePath(filepath);
	const std::string contentPath = ContentPathUtils::ToAbsoluteContentPath(relativeFilePath);

	tinyxml2::XMLDocument document;
	tinyxml2::XMLElement* root = document.NewElement("GameAsset");
	root->SetAttribute("FileType", kNavigationTreeFileType);
	document.InsertFirstChild(root);

	const NavigationNode* rootNode = navigationTree.GetRoot();
	root->SetAttribute("RootNodeId", rootNode ? rootNode->id : -1);

	tinyxml2::XMLElement* nodesElement = document.NewElement("Nodes");
	root->InsertEndChild(nodesElement);

	for (const std::unique_ptr<NavigationNode>& node : navigationTree.GetNodes())
	{
		if (!node)
		{
			continue;
		}

		tinyxml2::XMLElement* nodeElement = document.NewElement("Node");
		nodeElement->SetAttribute("Id", node->id);
		WriteArea(document, nodeElement, node->area);

		tinyxml2::XMLElement* neighboursElement = document.NewElement("Neighbours");
		for (const NavigationNode* neighbour : node->neighbours)
		{
			if (!neighbour)
			{
				continue;
			}

			tinyxml2::XMLElement* neighbourElement = document.NewElement("Neighbour");
			neighbourElement->SetAttribute("Id", neighbour->id);
			neighboursElement->InsertEndChild(neighbourElement);
		}
		nodeElement->InsertEndChild(neighboursElement);

		nodesElement->InsertEndChild(nodeElement);
	}

	return document.SaveFile(contentPath.c_str()) == tinyxml2::XML_SUCCESS;
}

bool NavigationTreeSerializer::Deserialize(const std::string& filepath, NavigationTree& outNavigationTree)
{
	outNavigationTree.GetNodes().clear();

	const std::string relativeFilePath = ContentPathUtils::ToContentRelativePath(filepath);
	const std::string contentPath = ContentPathUtils::ToAbsoluteContentPath(relativeFilePath);

	std::string fileContents;
	if (!DataEncryption::ReadTextFile(contentPath, fileContents))
	{
		return false;
	}

	tinyxml2::XMLDocument document;
	if (document.Parse(fileContents.c_str(), fileContents.size()) != tinyxml2::XML_SUCCESS)
	{
		return false;
	}

	tinyxml2::XMLElement* root = document.FirstChildElement("GameAsset");
	const char* fileType = root ? root->Attribute("FileType") : nullptr;
	if (!root || !fileType || std::string(fileType) != kNavigationTreeFileType)
	{
		return false;
	}

	std::unordered_map<int, NavigationNode*> nodeBySerializedId;
	int nextNodeId = 0;

	tinyxml2::XMLElement* nodesElement = root->FirstChildElement("Nodes");
	for (tinyxml2::XMLElement* nodeElement = nodesElement ? nodesElement->FirstChildElement("Node") : nullptr;
		nodeElement;
		nodeElement = nodeElement->NextSiblingElement("Node"))
	{
		int serializedId = -1;
		nodeElement->QueryIntAttribute("Id", &serializedId);
		NavigationNode* node = outNavigationTree.AddNode(ReadArea(nodeElement));
		node->id = serializedId;
		nodeBySerializedId[serializedId] = node;
		nextNodeId = GoknarMath::Max(nextNodeId, serializedId + 1);
	}
	outNavigationTree.EnsureNextNodeId(nextNodeId);

	for (tinyxml2::XMLElement* nodeElement = nodesElement ? nodesElement->FirstChildElement("Node") : nullptr;
		nodeElement;
		nodeElement = nodeElement->NextSiblingElement("Node"))
	{
		int serializedId = -1;
		nodeElement->QueryIntAttribute("Id", &serializedId);

		auto nodeIterator = nodeBySerializedId.find(serializedId);
		if (nodeIterator == nodeBySerializedId.end())
		{
			continue;
		}

		NavigationNode* node = nodeIterator->second;
		tinyxml2::XMLElement* neighboursElement = nodeElement->FirstChildElement("Neighbours");
		for (tinyxml2::XMLElement* neighbourElement = neighboursElement ? neighboursElement->FirstChildElement("Neighbour") : nullptr;
			neighbourElement;
			neighbourElement = neighbourElement->NextSiblingElement("Neighbour"))
		{
			int neighbourId = -1;
			neighbourElement->QueryIntAttribute("Id", &neighbourId);
			auto neighbourIterator = nodeBySerializedId.find(neighbourId);
			if (neighbourIterator != nodeBySerializedId.end())
			{
				node->neighbours.push_back(neighbourIterator->second);
			}
		}
	}

	int rootNodeId = -1;
	root->QueryIntAttribute("RootNodeId", &rootNodeId);
	auto rootNodeIterator = nodeBySerializedId.find(rootNodeId);
	outNavigationTree.SetRoot(rootNodeIterator != nodeBySerializedId.end() ? rootNodeIterator->second : outNavigationTree.GetRoot());

	return true;
}
