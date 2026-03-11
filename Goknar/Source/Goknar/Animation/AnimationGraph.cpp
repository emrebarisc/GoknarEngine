#include "pch.h"

#include "AnimationGraph.h"
#include "Goknar/Animation/AnimationNode.h"
#include "Goknar/Animation/AnimationState.h"
#include "Goknar/Animation/AnimationTransition.h"
#include "Goknar/Model/SkeletalMeshInstance.h"

void AnimationGraph::Init()
{
	PlayCurrentStateAnimation();
}

void AnimationGraph::PlayCurrentStateAnimation()
{
	PlayLoopData playLoopData;

	if (!currentState_->currentNode_->loop)
	{
		playLoopData.playOnce = true;
		playLoopData.callback = Delegate<void()>::Create<AnimationGraph, &AnimationGraph::SetIsCurrentStateAnimationFinishedTrue>(this);
	}

	relativeSkeletalMeshInstance->PlayAnimation(currentState_->currentNode_->animationName, playLoopData);
}

void AnimationGraph::Update(float deltaTime)
{
	if (!isDirty_)
	{
		return;
	}

	if (!currentState_.get())
	{
		return;
	}

	if (!relativeSkeletalMeshInstance)
	{
		return;
	}

	for (const auto& connection : currentState_->outboundConnections)
	{
		if (connection->ShouldTransit(this, false))
		{
			currentState_ = connection->target;
			currentState_->Reset();
			PlayCurrentStateAnimation();
		}
	}

	for (const auto& connection : currentState_->currentNode_->outboundConnections)
	{
		if (connection->ShouldTransit(this, isCurrentStateAnimationFinished))
		{
			currentState_->currentNode_ = connection->target;

			isCurrentStateAnimationFinished = false;

			PlayCurrentStateAnimation();

			break;
		}
	}

	if (0 < triggersToClear.size())
	{
		for (const auto& triggerToClear : triggersToClear)
		{
			SetVariable(triggerToClear, false);
		}

		triggersToClear.clear();
	}

	isDirty_ = false;
}