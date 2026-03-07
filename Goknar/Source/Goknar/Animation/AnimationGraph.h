#ifndef __ANIMATIONGRAPH_H__
#define __ANIMATIONGRAPH_H__

#include <memory>
#include <unordered_map>

#include "Goknar/Core.h"

#include "Goknar/Animation/AnimationTypes.h"

struct AnimationState;
struct SkeletalMeshInstance;

struct GOKNAR_API AnimationGraph
{
	AnimationGraph() = default;
	~AnimationGraph() = default;

	void Init();
	void Update(float deltaTime);

	const AnimationVariable* GetVariable(const std::string& name) const
	{
		std::unordered_map<std::string, AnimationVariable>::const_iterator it = variables.find(name);
		if (it != variables.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	template<typename T>
	void SetVariable(const std::string& name, T value)
	{
		variables[name] = value;
	}

	template<typename T, typename = typename std::enable_if<std::is_same<T, bool>::value>::type>
	void SetTrigger(const std::string& name, T value)
	{
		variables[name] = value;
		triggersToClear.emplace_back(name);
	}

	std::unordered_map<std::string, AnimationVariable> variables;

	std::shared_ptr<AnimationState> currentState = nullptr;

	SkeletalMeshInstance* relativeSkeletalMeshInstance = nullptr;

	bool isCurrentStateAnimationFinished = false;

private:
	inline void SetIsCurrentStateAnimationFinishedTrue()
	{
		isCurrentStateAnimationFinished = true;
	}

	std::vector<std::string> triggersToClear;
};

#endif