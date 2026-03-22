#include "pch.h"
#include "AnimationSerializer.h"

#include <filesystem>

#include "Goknar/Animation/AnimationNode.h"
#include "Goknar/Animation/AnimationCondition.h"

bool AnimationSerializer::Serialize(const AnimationGraph* graph, const std::string& filepath)
{
    std::string contentPath = ContentDir + filepath;

    if (!graph)
    {
        return false;
    }

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement* root = doc.NewElement("GameAsset");
    root->SetAttribute("FileType", "AnimationGraph");
    doc.InsertEndChild(root);

    tinyxml2::XMLElement* variablesEl = doc.NewElement("Variables");
    for (const auto& pair : graph->variables)
    {
        tinyxml2::XMLElement* varEl = doc.NewElement("Variable");
        varEl->SetAttribute("name", pair.first.c_str());
        SerializeVariable(varEl, pair.second);
        variablesEl->InsertEndChild(varEl);
    }
    root->InsertEndChild(variablesEl);

    tinyxml2::XMLElement* statesEl = doc.NewElement("States");
    for (const auto& state : graph->GetStates())
    {
        SerializeState(&doc, statesEl, state);
    }
    root->InsertEndChild(statesEl);

    std::filesystem::path pathObj(contentPath);
    std::filesystem::path directory = pathObj.parent_path();

    if (!directory.empty() && !std::filesystem::exists(directory))
    {
        std::error_code ec;
        std::filesystem::create_directories(directory, ec);
        if (ec) return false;
    }

    return doc.SaveFile(contentPath.c_str()) == tinyxml2::XML_SUCCESS;
}

int AnimationSerializer::GetNodeId(const AnimationNode* node)
{
    if (!node) return 0;
    if (nodeToIdMap_.find(node) == nodeToIdMap_.end())
    {
        nodeToIdMap_[node] = nextNodeId_++;
    }
    return nodeToIdMap_[node];
}

int AnimationSerializer::GetStateId(const AnimationState* state)
{
    if (!state) return 0;
    if (stateToIdMap_.find(state) == stateToIdMap_.end())
    {
        stateToIdMap_[state] = nextStateId_++;
    }
    return stateToIdMap_[state];
}

std::string AnimationSerializer::CompareOpToString(CompareOp op)
{
    switch (op)
    {
    case CompareOp::Equal: return "Equal";
    case CompareOp::NotEqual: return "NotEqual";
    case CompareOp::Greater: return "Greater";
    case CompareOp::Less: return "Less";
    case CompareOp::GreaterOrEqual: return "GreaterOrEqual";
    case CompareOp::LessOrEqual: return "LessOrEqual";
    default: return "Unknown";
    }
}

void AnimationSerializer::SerializeVariable(tinyxml2::XMLElement* element, const AnimationVariable& var)
{
    std::visit([element](const auto& value)
        {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, bool>)
            {
                element->SetAttribute("type", "bool"); element->SetAttribute("value", value);
            }
            else if constexpr (std::is_same_v<T, int>)
            {
                element->SetAttribute("type", "int"); element->SetAttribute("value", value);
            }
            else if constexpr (std::is_same_v<T, float>)
            {
                element->SetAttribute("type", "float"); element->SetAttribute("value", value);
            }
            else if constexpr (std::is_same_v<T, Vector2>)
            {
                element->SetAttribute("type", "Vector2"); element->SetAttribute("x", value.x); element->SetAttribute("y", value.y);
            }
            else if constexpr (std::is_same_v<T, Vector2i>)
            {
                element->SetAttribute("type", "Vector2i"); element->SetAttribute("x", value.x); element->SetAttribute("y", value.y);
            }
            else if constexpr (std::is_same_v<T, Vector3>)
            {
                element->SetAttribute("type", "Vector3"); element->SetAttribute("x", value.x); element->SetAttribute("y", value.y); element->SetAttribute("z", value.z);
            }
            else if constexpr (std::is_same_v<T, Vector3i>)
            {
                element->SetAttribute("type", "Vector3i"); element->SetAttribute("x", value.x); element->SetAttribute("y", value.y); element->SetAttribute("z", value.z);
            }
            else if constexpr (std::is_same_v<T, Vector4>)
            {
                element->SetAttribute("type", "Vector4"); element->SetAttribute("x", value.x); element->SetAttribute("y", value.y); element->SetAttribute("z", value.z); element->SetAttribute("w", value.w);
            }
        }, var);
}

void AnimationSerializer::CollectNodes(const std::shared_ptr<AnimationNode>& node, std::unordered_set<const AnimationNode*>& visited, std::vector<std::shared_ptr<AnimationNode>>& outNodes)
{
    if (!node || visited.count(node.get())) return;

    visited.insert(node.get());
    outNodes.push_back(node);

    for (const auto& transition : node->outboundConnections)
    {
        CollectNodes(transition->target, visited, outNodes);
    }
}

void AnimationSerializer::SerializeState(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, const std::shared_ptr<AnimationState>& state)
{
    tinyxml2::XMLElement* stateEl = doc->NewElement("State");
    stateEl->SetAttribute("id", GetStateId(state.get()));
    stateEl->SetAttribute("name", state->name.c_str());

    tinyxml2::XMLElement* stateOutboundsEl = doc->NewElement("OutboundConnections");
    for (const auto& transition : state->outboundConnections)
    {
        SerializeStateTransition(doc, stateOutboundsEl, transition);
    }
    stateEl->InsertEndChild(stateOutboundsEl);

    std::shared_ptr<AnimationNode> entryNode = state->GetEntryNode();
    if (entryNode)
    {
        tinyxml2::XMLElement* entryEl = doc->NewElement("EntryNode");
        entryEl->SetAttribute("id", GetNodeId(entryNode.get()));
        stateEl->InsertEndChild(entryEl);
    }

    std::unordered_set<const AnimationNode*> visited;
    std::vector<std::shared_ptr<AnimationNode>> flatNodes;
    CollectNodes(entryNode, visited, flatNodes);

    tinyxml2::XMLElement* nodesEl = doc->NewElement("Nodes");
    for (const auto& node : flatNodes)
    {
        SerializeNode(doc, nodesEl, node);
    }
    stateEl->InsertEndChild(nodesEl);

    parent->InsertEndChild(stateEl);
}

void AnimationSerializer::SerializeNode(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, const std::shared_ptr<AnimationNode>& node)
{
    tinyxml2::XMLElement* nodeEl = doc->NewElement("Node");
    nodeEl->SetAttribute("id", GetNodeId(node.get()));
    nodeEl->SetAttribute("animationName", node->animationName.c_str());
    nodeEl->SetAttribute("loop", node->loop);

    tinyxml2::XMLElement* outboundsEl = doc->NewElement("OutboundConnections");
    for (const auto& transition : node->outboundConnections)
    {
        SerializeNodeTransition(doc, outboundsEl, transition);
    }
    nodeEl->InsertEndChild(outboundsEl);

    parent->InsertEndChild(nodeEl);
}

void AnimationSerializer::SerializeNodeTransition(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, const std::shared_ptr<AnimationTransition<AnimationNode>>& transition)
{
    tinyxml2::XMLElement* transEl = doc->NewElement("Transition");
    transEl->SetAttribute("targetId", GetNodeId(transition->target.get()));
    transEl->SetAttribute("transitWhenAnimationDone", transition->transitWhenAnimationDone);

    tinyxml2::XMLElement* conditionsEl = doc->NewElement("Conditions");
    for (const auto& condition : transition->conditions)
    {
        tinyxml2::XMLElement* condEl = doc->NewElement("Condition");
        condEl->SetAttribute("variableName", condition.variableName.c_str());
        condEl->SetAttribute("operation", CompareOpToString(condition.operation).c_str());
        SerializeVariable(condEl, condition.targetValue);
        conditionsEl->InsertEndChild(condEl);
    }
    transEl->InsertEndChild(conditionsEl);

    parent->InsertEndChild(transEl);
}

void AnimationSerializer::SerializeStateTransition(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, const std::shared_ptr<AnimationTransition<AnimationState>>& transition)
{
    tinyxml2::XMLElement* transEl = doc->NewElement("Transition");
    transEl->SetAttribute("targetId", GetStateId(transition->target.get()));
    transEl->SetAttribute("transitWhenAnimationDone", transition->transitWhenAnimationDone);

    tinyxml2::XMLElement* conditionsEl = doc->NewElement("Conditions");
    for (const auto& condition : transition->conditions)
    {
        tinyxml2::XMLElement* condEl = doc->NewElement("Condition");
        condEl->SetAttribute("variableName", condition.variableName.c_str());
        condEl->SetAttribute("operation", CompareOpToString(condition.operation).c_str());
        SerializeVariable(condEl, condition.targetValue);
        conditionsEl->InsertEndChild(condEl);
    }
    transEl->InsertEndChild(conditionsEl);

    parent->InsertEndChild(transEl);
}