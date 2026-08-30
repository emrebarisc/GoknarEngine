#include "pch.h"

#include "AnimationGraph.h"
#include "Goknar/Animation/AnimationNode.h"
#include "Goknar/Animation/AnimationState.h"
#include "Goknar/Animation/AnimationTransition.h"
#include "Goknar/Model/SkeletalMeshInstance.h"

void AnimationGraph::Init()
{
	if (currentState_)
	{
		currentState_->Reset();
		ResetNodeRuntime(currentState_->currentNode_);
	}

	if (relativeSkeletalMeshInstance)
	{
		relativeSkeletalMeshInstance->EvaluateAnimationGraph(*this, 0.f);
	}
}

void AnimationGraph::Update(float deltaTime)
{
	if (!currentState_.get())
	{
		return;
	}

	if (!relativeSkeletalMeshInstance)
	{
		return;
	}

	if (!IsCrossfading())
	{
		for (const auto& connection : currentState_->outboundConnections)
		{
			if (connection->ShouldTransit(this, false))
			{
				if (const auto target = connection->target.lock())
				{
					StartStateTransition(target, connection->duration);
					break;
				}
			}
		}

		const bool currentNodeFinished = currentState_->currentNode_ ?
			GetRuntimeData(currentState_->currentNode_.get()).finished :
			false;

		if (currentState_->currentNode_)
		{
			for (const auto& connection : currentState_->currentNode_->outboundConnections)
			{
				if (connection->ShouldTransit(this, currentNodeFinished))
				{
					const auto target = connection->target.lock();
					if (!target)
					{
						continue;
					}

					StartNodeTransition(target, connection->duration);
					break;
				}
			}
		}
	}

	if (IsCrossfading())
	{
		crossfadeElapsed_ += deltaTime;
		if (crossfadeDuration_ <= crossfadeElapsed_)
		{
			crossfadeSourceNode_ = nullptr;
			crossfadeDuration_ = 0.f;
			crossfadeElapsed_ = 0.f;
		}
	}

	relativeSkeletalMeshInstance->EvaluateAnimationGraph(*this, deltaTime);


	if (0 < triggersToClear.size())
	{
		for (const auto& triggerToClear : triggersToClear)
		{
			SetVariable(triggerToClear, false);
		}

		triggersToClear.clear();
	}
}

void AnimationGraph::ResetNodeRuntime(const std::shared_ptr<AnimationNode>& node)
{
	if (!node)
	{
		return;
	}

	nodeRuntimeData_[node.get()] = AnimationNodeRuntimeData();
}

void AnimationGraph::StartNodeTransition(const std::shared_ptr<AnimationNode>& target, float duration)
{
	if (!currentState_ || !target || currentState_->currentNode_ == target)
	{
		return;
	}

	crossfadeSourceNode_ = currentState_->currentNode_;
	crossfadeDuration_ = GoknarMath::Max(duration, 0.f);
	crossfadeElapsed_ = 0.f;

	currentState_->currentNode_ = target;
	ResetNodeRuntime(target);
	isCurrentStateAnimationFinished = false;
}

void AnimationGraph::StartStateTransition(const std::shared_ptr<AnimationState>& target, float duration)
{
	if (!target || currentState_ == target)
	{
		return;
	}

	crossfadeSourceNode_ = currentState_ ? currentState_->currentNode_ : nullptr;
	crossfadeDuration_ = GoknarMath::Max(duration, 0.f);
	crossfadeElapsed_ = 0.f;

	SetCurrentState(target);
	ResetNodeRuntime(currentState_->currentNode_);
	isCurrentStateAnimationFinished = false;
}

float AnimationGraph::GetFloatVariable(const std::string& name, float fallback) const
{
	const AnimationVariable* variable = GetVariable(name);
	if (!variable)
	{
		return fallback;
	}

	if (const float* value = std::get_if<float>(variable))
	{
		return *value;
	}

	if (const int* value = std::get_if<int>(variable))
	{
		return (float)*value;
	}

	return fallback;
}

bool AnimationGraph::GetBoolVariable(const std::string& name, bool fallback) const
{
	const AnimationVariable* variable = GetVariable(name);
	if (!variable)
	{
		return fallback;
	}

	if (const bool* value = std::get_if<bool>(variable))
	{
		return *value;
	}

	return fallback;
}
