#include "pch.h"
#include "AnimationDeserializer.h"

#include "Goknar/Animation/AnimationNode.h"
#include "Goknar/Animation/AnimationTransition.h"
#include "Goknar/Animation/AnimationCondition.h"

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
    if (doc.LoadFile(contentPath.c_str()) != tinyxml2::XML_SUCCESS)
    {
        return false;
    }

    // Look for the generic "FileType" root element
    tinyxml2::XMLElement* root = doc.FirstChildElement("FileType");
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
                node->animationName = nodeEl->Attribute("animationName");
                node->loop = nodeEl->BoolAttribute("loop");
                idToNodeMap_[nodeId] = node;
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

        tinyxml2::XMLElement* entryEl = stateEl->FirstChildElement("EntryNode");
        if (entryEl)
        {
            int entryId = entryEl->IntAttribute("id");
            state->SetEntryNode(idToNodeMap_[entryId]);
        }
    }

    if (!graph->GetStates().empty())
    {
        graph->SetCurrentState(graph->GetStates().front());
    }

    return true;
}