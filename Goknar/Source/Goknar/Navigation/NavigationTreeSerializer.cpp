#include "NavigationTreeSerializer.h"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>

#include "tinyxml2.h"

#include "Goknar/Data/DataEncryption.h"
#include "Goknar/Helpers/ContentPathUtils.h"
#include "Goknar/Navigation/NavigationTree.h"

namespace
{
	constexpr const char* kNavigationTreeFileType = "NavigationTree";

	void WriteVector3Values(std::ostringstream& stream, const Vector3& value)
	{
		stream << value.x << "," << value.y << "," << value.z;
	}

	void WriteArea(tinyxml2::XMLDocument& document, tinyxml2::XMLElement* nodeElement, const Area& area)
	{
		tinyxml2::XMLElement* areaElement = document.NewElement("Area");
		tinyxml2::XMLElement* pointsElement = document.NewElement("Points");

		const Vector3 points[4] = { area.point0, area.point1, area.point2, area.point3 };
		std::ostringstream pointsStream;
		pointsStream << std::setprecision(std::numeric_limits<float>::max_digits10);

		for (int pointIndex = 0; pointIndex < 4; ++pointIndex)
		{
			if (pointIndex > 0)
			{
				pointsStream << " ";
			}

			WriteVector3Values(pointsStream, points[pointIndex]);
		}

		const std::string pointsText = pointsStream.str();
		pointsElement->SetAttribute("V", pointsText.c_str());
		areaElement->InsertEndChild(pointsElement);
		nodeElement->InsertEndChild(areaElement);
	}

	Area ReadArea(const tinyxml2::XMLElement* nodeElement)
	{
		Area area;
		const tinyxml2::XMLElement* areaElement = nodeElement ? nodeElement->FirstChildElement("Area") : nullptr;
		const tinyxml2::XMLElement* pointsElement = areaElement ? areaElement->FirstChildElement("Points") : nullptr;
		const char* pointsValue = pointsElement ? pointsElement->Attribute("V") : nullptr;
		if (!pointsValue)
		{
			return area;
		}

		std::string pointsText = pointsValue;
		std::replace(pointsText.begin(), pointsText.end(), ',', ' ');

		std::stringstream pointsStream(pointsText);
		Vector3 points[4] = { Vector3::ZeroVector, Vector3::ZeroVector, Vector3::ZeroVector, Vector3::ZeroVector };
		for (int pointIndex = 0; pointIndex < 4; ++pointIndex)
		{
			if (!(pointsStream >> points[pointIndex].x >> points[pointIndex].y >> points[pointIndex].z))
			{
				break;
			}
		}

		area.point0 = points[0];
		area.point1 = points[1];
		area.point2 = points[2];
		area.point3 = points[3];

		return area;
	}

	void AddNeighbourBySerializedId(
		NavigationNode* node,
		const std::unordered_map<int, NavigationNode*>& nodeBySerializedId,
		int neighbourId)
	{
		auto neighbourIterator = nodeBySerializedId.find(neighbourId);
		if (neighbourIterator != nodeBySerializedId.end())
		{
			node->neighbours.push_back(neighbourIterator->second);
		}
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

		std::string neighbourIds;
		for (const NavigationNode* neighbour : node->neighbours)
		{
			if (!neighbour)
			{
				continue;
			}

			if (!neighbourIds.empty())
			{
				neighbourIds += ",";
			}

			neighbourIds += std::to_string(neighbour->id);
		}

		if (!neighbourIds.empty())
		{
			tinyxml2::XMLElement* neighboursElement = document.NewElement("Neighbours");
			neighboursElement->SetAttribute("Ids", neighbourIds.c_str());
			nodeElement->InsertEndChild(neighboursElement);
		}

		nodesElement->InsertEndChild(nodeElement);
	}

	return document.SaveFile(contentPath.c_str()) == tinyxml2::XML_SUCCESS;
}

bool NavigationTreeSerializer::Deserialize(const std::string& filepath, NavigationTree& outNavigationTree)
{
	outNavigationTree.Clear();

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
		const char* neighbourIds = neighboursElement ? neighboursElement->Attribute("Ids") : nullptr;

		if (!neighbourIds)
		{
			continue;
		}

		std::stringstream neighbourIdsStream(neighbourIds);
		std::string neighbourIdText;
		while (std::getline(neighbourIdsStream, neighbourIdText, ','))
		{
			std::stringstream neighbourIdStream(neighbourIdText);
			int neighbourId = -1;
			if (neighbourIdStream >> neighbourId)
			{
				AddNeighbourBySerializedId(node, nodeBySerializedId, neighbourId);
			}
		}
	}

	int rootNodeId = -1;
	root->QueryIntAttribute("RootNodeId", &rootNodeId);
	auto rootNodeIterator = nodeBySerializedId.find(rootNodeId);
	outNavigationTree.SetRoot(rootNodeIterator != nodeBySerializedId.end() ? rootNodeIterator->second : outNavigationTree.GetRoot());

	return true;
}
