#ifndef __ANIMATIONSERIALIZER_H__
#define __ANIMATIONSERIALIZER_H__

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Goknar/Animation/AnimationGraph.h"
#include "Goknar/Animation/AnimationTransition.h"
#include <tinyxml2.h>

class GOKNAR_API AnimationSerializer
{
public:
    AnimationSerializer() = default;
    ~AnimationSerializer() = default;

    bool Serialize(const AnimationGraph* graph, const std::string& filepath);

private:
    int nextNodeId_{ 1 };
    std::unordered_map<const AnimationNode*, int> nodeToIdMap_;

    int nextStateId_{ 1 };
    std::unordered_map<const AnimationState*, int> stateToIdMap_;

    int GetNodeId(const AnimationNode* node);
    int GetStateId(const AnimationState* state);
    std::string CompareOpToString(CompareOp op);

    void SerializeVariable(tinyxml2::XMLElement* element, const AnimationVariable& var);
    void SerializeState(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, const std::shared_ptr<AnimationState>& state);
    void SerializeNode(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, const std::shared_ptr<AnimationNode>& node);

    void SerializeNodeTransition(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, const std::shared_ptr<AnimationTransition<AnimationNode>>& transition);
    void SerializeStateTransition(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* parent, const std::shared_ptr<AnimationTransition<AnimationState>>& transition);

    void CollectNodes(const std::shared_ptr<AnimationNode>& node, std::unordered_set<const AnimationNode*>& visited, std::vector<std::shared_ptr<AnimationNode>>& outNodes);
};

#endif