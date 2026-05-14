#ifndef __ANIMATIONGRAPH_H__
#define __ANIMATIONGRAPH_H__

#include <memory>
#include <unordered_map>

#include "Goknar/Core.h"

#include "Goknar/Animation/AnimationNode.h"
#include "Goknar/Animation/AnimationState.h"
#include "Goknar/Animation/AnimationTypes.h"

class SkeletalMeshInstance;

struct GOKNAR_API AnimationNodeRuntimeData
{
	float time{ 0.f };
	float normalizedTime{ 0.f };
	float smoothedParameter{ 0.f };
	float smoothedParameterX{ 0.f };
	float smoothedParameterY{ 0.f };
	bool hasSmoothedParameters{ false };
	bool finished{ false };
};

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

	void SetVariable(const std::string& name, const AnimationVariable& value)
	{
		variables[name] = value;
	}

	template<typename T, typename = typename std::enable_if<std::is_same<T, bool>::value>::type>
	void SetTrigger(const std::string& name, T value)
	{
		variables[name] = value;
		triggersToClear.emplace_back(name);
	}

	const std::shared_ptr<AnimationState>& GetCurrentState() const
	{
		return currentState_;
	}

	void SetCurrentState(const std::shared_ptr<AnimationState>& currentState)
	{
		currentState_ = currentState;
		currentState_->Reset();
	}

	void AddState(const std::shared_ptr<AnimationState>& state)
	{
		states_.push_back(state);
	}

	const std::vector<std::shared_ptr<AnimationState>>& GetStates() const
	{
		return states_;
	}

	const std::shared_ptr<AnimationNode>& GetCurrentNode() const
	{
		static std::shared_ptr<AnimationNode> nullNode{ nullptr };
		return currentState_ ? currentState_->currentNode_ : nullNode;
	}

	const std::shared_ptr<AnimationNode>& GetCrossfadeSourceNode() const
	{
		return crossfadeSourceNode_;
	}

	bool IsCrossfading() const
	{
		return crossfadeSourceNode_ && crossfadeDuration_ > 0.f && crossfadeElapsed_ < crossfadeDuration_;
	}

	float GetCrossfadeAlpha() const
	{
		if (crossfadeDuration_ <= 0.f)
		{
			return 1.f;
		}

		return GoknarMath::Clamp(crossfadeElapsed_ / crossfadeDuration_, 0.f, 1.f);
	}

	AnimationNodeRuntimeData& GetRuntimeData(const AnimationNode* node)
	{
		return nodeRuntimeData_[node];
	}

	const AnimationNodeRuntimeData* GetRuntimeData(const AnimationNode* node) const
	{
		const auto iterator = nodeRuntimeData_.find(node);
		return iterator != nodeRuntimeData_.end() ? &iterator->second : nullptr;
	}

	float GetFloatVariable(const std::string& name, float fallback = 0.f) const;
	bool GetBoolVariable(const std::string& name, bool fallback = false) const;

	std::unordered_map<std::string, AnimationVariable> variables;

	SkeletalMeshInstance* relativeSkeletalMeshInstance = nullptr;

	bool isCurrentStateAnimationFinished = false;

private:
	void ResetNodeRuntime(const std::shared_ptr<AnimationNode>& node);
	void StartNodeTransition(const std::shared_ptr<AnimationNode>& target, float duration);
	void StartStateTransition(const std::shared_ptr<AnimationState>& target, float duration);

	inline void SetIsCurrentStateAnimationFinishedTrue()
	{
		isCurrentStateAnimationFinished = true;
	}

	std::shared_ptr<AnimationState> currentState_{ nullptr };
	std::vector<std::shared_ptr<AnimationState>> states_{};
	std::unordered_map<const AnimationNode*, AnimationNodeRuntimeData> nodeRuntimeData_{};
	std::shared_ptr<AnimationNode> crossfadeSourceNode_{ nullptr };
	float crossfadeDuration_{ 0.f };
	float crossfadeElapsed_{ 0.f };

	std::vector<std::string> triggersToClear{};
};

#endif
