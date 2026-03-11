#ifndef __ANIMATIONDESERIALIZER_H__
#define __ANIMATIONDESERIALIZER_H__

#include <string>
#include <unordered_map>
#include <memory>

#include "Goknar/Animation/AnimationGraph.h"
#include <tinyxml2.h>

class GOKNAR_API AnimationDeserializer
{
public:
    AnimationDeserializer() = default;
    ~AnimationDeserializer() = default;

    bool Deserialize(AnimationGraph* graph, const std::string& filepath);

private:
    std::unordered_map<int, std::shared_ptr<AnimationNode>> idToNodeMap_;
    std::unordered_map<int, std::shared_ptr<AnimationState>> idToStateMap_;

    CompareOp StringToCompareOp(const std::string& opStr);
    AnimationVariable DeserializeVariable(tinyxml2::XMLElement* element);
};

#endif