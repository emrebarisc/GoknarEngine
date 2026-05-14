#include "pch.h"
#include "AnimationDeserializer.h"

#include "Goknar/Animation/AnimationNode.h"
#include "Goknar/Animation/AnimationTransition.h"
#include "Goknar/Animation/AnimationCondition.h"
#include "Goknar/Data/DataEncryption.h"

namespace
{
	AnimationNodeType StringToAnimationNodeType(const char* type)
	{
		if (!type)
		{
			return AnimationNodeType::Clip;
		}

		const std::string typeString = type;
		if (typeString == "BlendSpace1D" || typeString == "BlendSpace1DNode")
		{
			return AnimationNodeType::BlendSpace1D;
		}
		if (typeString == "BlendSpace2D" || typeString == "BlendSpace2DNode")
		{
			return AnimationNodeType::BlendSpace2D;
		}

		return AnimationNodeType::Clip;
	}

	const char* FirstAttribute(tinyxml2::XMLElement* element, const char* first, const char* second)
	{
		if (!element)
		{
			return nullptr;
		}

		const char* value = element->Attribute(first);
		return value ? value : element->Attribute(second);
	}

	float ReadFloatAttribute(tinyxml2::XMLElement* element, const char* name, float fallback)
	{
		return element && element->Attribute(name) ? element->FloatAttribute(name) : fallback;
	}
}

CompareOp AnimationDeserializer::StringToCompareOp(const std::string& opStr)
{
    if (opStr == "Equal") return CompareOp::Equal;
    if (opStr == "NotEqual") return CompareOp::NotEqual;
    if (opStr == "Greater") return CompareOp::Greater;
    if (opStr == "Less") return CompareOp::Less;
    if (opStr == "GreaterOrEqual") return CompareOp::GreaterOrEqual;
    if (opStr == "LessOrEqual") return CompareOp::LessOrEqual;
    return CompareOp::Equal;
}

AnimationVariable AnimationDeserializer::DeserializeVariable(tinyxml2::XMLElement* element)
{
    std::string type = element->Attribute("type");

    if (type == "bool") return element->BoolAttribute("value");
    if (type == "int") return element->IntAttribute("value");
    if (type == "float") return element->FloatAttribute("value");
    if (type == "Vector2") return Vector2{ element->FloatAttribute("x"), element->FloatAttribute("y") };
    if (type == "Vector2i") return Vector2i{ element->IntAttribute("x"), element->IntAttribute("y") };
    if (type == "Vector3") return Vector3{ element->FloatAttribute("x"), element->FloatAttribute("y"), element->FloatAttribute("z") };
    if (type == "Vector3i") return Vector3i{ element->IntAttribute("x"), element->IntAttribute("y"), element->IntAttribute("z") };
    if (type == "Vector4") return Vector4{ element->FloatAttribute("x"), element->FloatAttribute("y"), element->FloatAttribute("z"), element->FloatAttribute("w") };

    return false;
}

bool AnimationDeserializer::Deserialize(AnimationGraph* graph, const std::string& filepath)
{
    std::string contentPath = ContentDir + filepath;

    if (!graph)
    {
        return false;
    }

    tinyxml2::XMLDocument doc;
    std::string fileContents;
    if (!DataEncryption::ReadTextFile(contentPath, fileContents) ||
        doc.Parse(fileContents.c_str(), fileContents.size()) != tinyxml2::XML_SUCCESS)
    {
        return false;
    }

    // Look for the generic "FileType" root element
    tinyxml2::XMLElement* root = doc.FirstChildElement("GameAsset");
    if (!root)
    {
        return false;
    }

    // Verify that the FileType attribute is "AnimationGraph"
    const char* fileTypeAttr = root->Attribute("FileType");
    if (!fileTypeAttr || std::string(fileTypeAttr) != "AnimationGraph")
    {
        return false;
    }

    idToNodeMap_.clear();
    idToStateMap_.clear();

    tinyxml2::XMLElement* variablesEl = root->FirstChildElement("Variables");
    if (variablesEl)
    {
        for (tinyxml2::XMLElement* varEl = variablesEl->FirstChildElement("Variable"); varEl != nullptr; varEl = varEl->NextSiblingElement("Variable"))
        {
            std::string name = varEl->Attribute("name");
            graph->SetVariable(name, DeserializeVariable(varEl));
        }
    }

    tinyxml2::XMLElement* statesEl = root->FirstChildElement("States");
    if (!statesEl) return true;

   
    for (tinyxml2::XMLElement* stateEl = statesEl->FirstChildElement("State"); stateEl != nullptr; stateEl = stateEl->NextSiblingElement("State"))
    {
        int stateId = stateEl->IntAttribute("id");
        auto state = std::make_shared<AnimationState>();
        state->name = stateEl->Attribute("name");
        idToStateMap_[stateId] = state;

        graph->AddState(state);

        tinyxml2::XMLElement* nodesEl = stateEl->FirstChildElement("Nodes");
        if (nodesEl)
        {
            for (tinyxml2::XMLElement* nodeEl = nodesEl->FirstChildElement("Node"); nodeEl != nullptr; nodeEl = nodeEl->NextSiblingElement("Node"))
            {
                int nodeId = nodeEl->IntAttribute("id");
                auto node = std::make_shared<AnimationNode>();
                node->type = StringToAnimationNodeType(nodeEl->Attribute("type"));

                if (const char* animationName = FirstAttribute(nodeEl, "animationName", "clip"))
                {
                    node->animationName = animationName;
                }
                if (const char* parameterName = FirstAttribute(nodeEl, "parameterName", "parameter"))
                {
                    node->parameterName = parameterName;
                }
                if (const char* parameterXName = FirstAttribute(nodeEl, "parameterXName", "parameterX"))
                {
                    node->parameterXName = parameterXName;
                }
                if (const char* parameterYName = FirstAttribute(nodeEl, "parameterYName", "parameterY"))
                {
                    node->parameterYName = parameterYName;
                }
                if (const char* syncGroup = nodeEl->Attribute("syncGroup"))
                {
                    node->syncGroup = syncGroup;
                }

                node->loop = nodeEl->BoolAttribute("loop", true);
                node->playRate = ReadFloatAttribute(nodeEl, "playRate", node->playRate);
                node->parameterSmoothingSpeed = ReadFloatAttribute(nodeEl, "parameterSmoothingSpeed", node->parameterSmoothingSpeed);
                node->parameterSmoothingSpeed = ReadFloatAttribute(nodeEl, "smoothingSpeed", node->parameterSmoothingSpeed);

                tinyxml2::XMLElement* pointsParentEl = nodeEl->FirstChildElement("Points");
                if (!pointsParentEl)
                {
                    pointsParentEl = nodeEl;
                }

                for (tinyxml2::XMLElement* pointEl = pointsParentEl->FirstChildElement("Point"); pointEl != nullptr; pointEl = pointEl->NextSiblingElement("Point"))
                {
                    if (node->type == AnimationNodeType::BlendSpace1D)
                    {
                        BlendSpace1DPoint point;
                        point.value = pointEl->Attribute("value") ? pointEl->FloatAttribute("value") : pointEl->FloatAttribute("x");
                        if (const char* clipName = FirstAttribute(pointEl, "animationName", "clip"))
                        {
                            point.animationName = clipName;
                        }
                        node->blendSpace1DPoints.push_back(point);
                    }
                    else if (node->type == AnimationNodeType::BlendSpace2D)
                    {
                        BlendSpace2DPoint point;
                        point.x = pointEl->FloatAttribute("x");
                        point.y = pointEl->FloatAttribute("y");
                        if (const char* clipName = FirstAttribute(pointEl, "animationName", "clip"))
                        {
                            point.animationName = clipName;
                        }
                        node->blendSpace2DPoints.push_back(point);
                    }
                }

                idToNodeMap_[nodeId] = node;
                state->AddNode(node);
            }
        }
    }

   
    for (tinyxml2::XMLElement* stateEl = statesEl->FirstChildElement("State"); stateEl != nullptr; stateEl = stateEl->NextSiblingElement("State"))
    {
        int stateId = stateEl->IntAttribute("id");
        auto state = idToStateMap_[stateId];

        tinyxml2::XMLElement* stateOutboundsEl = stateEl->FirstChildElement("OutboundConnections");
        if (stateOutboundsEl)
        {
            for (tinyxml2::XMLElement* transEl = stateOutboundsEl->FirstChildElement("Transition"); transEl != nullptr; transEl = transEl->NextSiblingElement("Transition"))
            {
                auto transition = std::make_shared<AnimationTransition<AnimationState>>();
                int targetId = transEl->IntAttribute("targetId");

                transition->target = idToStateMap_[targetId];
                transition->transitWhenAnimationDone = transEl->BoolAttribute("transitWhenAnimationDone");
                transition->duration = ReadFloatAttribute(transEl, "duration", transition->duration);

                tinyxml2::XMLElement* conditionsEl = transEl->FirstChildElement("Conditions");
                if (conditionsEl)
                {
                    for (tinyxml2::XMLElement* condEl = conditionsEl->FirstChildElement("Condition"); condEl != nullptr; condEl = condEl->NextSiblingElement("Condition"))
                    {
                        AnimationCondition condition;
                        condition.variableName = condEl->Attribute("variableName");
                        condition.operation = StringToCompareOp(condEl->Attribute("operation"));
                        condition.targetValue = DeserializeVariable(condEl);
                        transition->conditions.push_back(condition);
                    }
                }
                state->outboundConnections.push_back(transition);
            }
        }

        tinyxml2::XMLElement* nodesEl = stateEl->FirstChildElement("Nodes");
        if (nodesEl)
        {
            for (tinyxml2::XMLElement* nodeEl = nodesEl->FirstChildElement("Node"); nodeEl != nullptr; nodeEl = nodeEl->NextSiblingElement("Node"))
            {
                int sourceId = nodeEl->IntAttribute("id");
                auto sourceNode = idToNodeMap_[sourceId];

                tinyxml2::XMLElement* outboundsEl = nodeEl->FirstChildElement("OutboundConnections");
                if (outboundsEl)
                {
                    for (tinyxml2::XMLElement* transEl = outboundsEl->FirstChildElement("Transition"); transEl != nullptr; transEl = transEl->NextSiblingElement("Transition"))
                    {
                        auto transition = std::make_shared<AnimationTransition<AnimationNode>>();
                        int targetId = transEl->IntAttribute("targetId");

                        transition->target = idToNodeMap_[targetId];
                        transition->transitWhenAnimationDone = transEl->BoolAttribute("transitWhenAnimationDone");
                        transition->duration = ReadFloatAttribute(transEl, "duration", transition->duration);

                        tinyxml2::XMLElement* conditionsEl = transEl->FirstChildElement("Conditions");
                        if (conditionsEl)
                        {
                            for (tinyxml2::XMLElement* condEl = conditionsEl->FirstChildElement("Condition"); condEl != nullptr; condEl = condEl->NextSiblingElement("Condition"))
                            {
                                AnimationCondition condition;
                                condition.variableName = condEl->Attribute("variableName");
                                condition.operation = StringToCompareOp(condEl->Attribute("operation"));
                                condition.targetValue = DeserializeVariable(condEl);
                                transition->conditions.push_back(condition);
                            }
                        }
                        sourceNode->outboundConnections.push_back(transition);
                    }
                }
            }
        }

        if (stateEl->Attribute("entryNodeId"))
        {
            const int entryId = stateEl->IntAttribute("entryNodeId");
            state->SetEntryNode(idToNodeMap_[entryId]);
        }
        else if (tinyxml2::XMLElement* entryEl = stateEl->FirstChildElement("EntryNode"))
        {
            int entryId = entryEl->IntAttribute("id");
            state->SetEntryNode(idToNodeMap_[entryId]);
        }
    }

    if (statesEl->Attribute("entryStateId"))
    {
        const int entryStateId = statesEl->IntAttribute("entryStateId");
        const auto iterator = idToStateMap_.find(entryStateId);
        if (iterator != idToStateMap_.end())
        {
            graph->SetCurrentState(iterator->second);
        }
    }
    else if (!graph->GetStates().empty())
    {
        graph->SetCurrentState(graph->GetStates().front());
    }

    return true;
}
