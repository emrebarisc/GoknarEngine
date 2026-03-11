#ifndef __ANIMATIONGRAPH_H__
#define __ANIMATIONGRAPH_H__

#include <memory>
#include <unordered_map>

#include "Goknar/Core.h"

#include "Goknar/Animation/AnimationState.h"
#include "Goknar/Animation/AnimationTypes.h"

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
		if (!isDirty_)
		{
			if (T* currentValue = std::get_if<T>(&variables[name]))
			{
				if (*currentValue == value)
				{
					return;
				}
			}
		}

		variables[name] = value;
		isDirty_ = true;
	}

	void SetVariable(const std::string& name, const AnimationVariable& value)
	{
		if (!isDirty_)
		{
			auto it = variables.find(name);
			if (it != variables.end() && it->second == value)
			{
				return;
			}
		}

		variables[name] = value;
		isDirty_ = true;
	}

	template<typename T, typename = typename std::enable_if<std::is_same<T, bool>::value>::type>
	void SetTrigger(const std::string& name, T value)
	{
		if (!isDirty_)
		{
			if (T* currentValue = std::get_if<T>(&variables[name]))
			{
				if (*currentValue == value)
				{
					return;
				}
			}
		}

		variables[name] = value;
		triggersToClear.emplace_back(name);
		isDirty_ = true;
	}

	const std::shared_ptr<AnimationState>& GetCurrentState() const
	{
		return currentState_;
	}

	void SetCurrentState(const std::shared_ptr<AnimationState>& currentState)
	{
		currentState_ = currentState;
		currentState_->Reset();
		Init();
	}

	void AddState(const std::shared_ptr<AnimationState>& state)
	{
		states_.push_back(state);
	}

	const std::vector<std::shared_ptr<AnimationState>>& GetStates() const
	{
		return states_;
	}

	std::unordered_map<std::string, AnimationVariable> variables;

	SkeletalMeshInstance* relativeSkeletalMeshInstance = nullptr;

	bool isCurrentStateAnimationFinished = false;

private:
	inline void SetIsCurrentStateAnimationFinishedTrue()
	{
		isCurrentStateAnimationFinished = true;
	}

	std::shared_ptr<AnimationState> currentState_{ nullptr };
	std::vector<std::shared_ptr<AnimationState>> states_{};

	std::vector<std::string> triggersToClear{};
	
	bool isDirty_{ false };
};

#endif