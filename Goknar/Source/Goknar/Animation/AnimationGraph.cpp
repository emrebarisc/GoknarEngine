#include "pch.h"

#include "AnimationGraph.h"
#include "Goknar/Animation/AnimationNode.h"
#include "Goknar/Animation/AnimationState.h"
#include "Goknar/Animation/AnimationNodeTransition.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Model/SkeletalMeshInstance.h"

void AnimationGraph::Init()
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

	if (!currentState_.get() && !relativeSkeletalMeshInstance)
	{
		return;
	}

	for (const auto& connection : currentState_->currentNode_->outboundConnections)
	{
		if (connection->ShouldTransit(this, isCurrentStateAnimationFinished))
		{
			currentState_->currentNode_ = connection->targetNode;

			isCurrentStateAnimationFinished = false;

			PlayLoopData playLoopData;

			if (!currentState_->currentNode_->loop)
			{
				playLoopData.playOnce = true;
				playLoopData.callback = Delegate<void()>::Create<AnimationGraph, &AnimationGraph::SetIsCurrentStateAnimationFinishedTrue>(this);
			}

			relativeSkeletalMeshInstance->PlayAnimation(currentState_->currentNode_->animationName, playLoopData);

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