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

	if (!currentState->currentNode->loop)
	{
		playLoopData.playOnce = true;
		playLoopData.callback = Delegate<void()>::Create<AnimationGraph, &AnimationGraph::SetIsCurrentStateAnimationFinishedTrue>(this);
	}

	relativeSkeletalMeshInstance->PlayAnimation(currentState->currentNode->animationName, playLoopData);
}

void AnimationGraph::Update(float deltaTime)
{
	if (!currentState.get() && !relativeSkeletalMeshInstance)
	{
		return;
	}

	for (const auto& connection : currentState->currentNode->outboundConnections)
	{
		if (connection->ShouldTransit(this, isCurrentStateAnimationFinished))
		{
			currentState->currentNode = connection->targetNode;

			isCurrentStateAnimationFinished = false;

			PlayLoopData playLoopData;

			if (!currentState->currentNode->loop)
			{
				playLoopData.playOnce = true;
				playLoopData.callback = Delegate<void()>::Create<AnimationGraph, &AnimationGraph::SetIsCurrentStateAnimationFinishedTrue>(this);
			}

			relativeSkeletalMeshInstance->PlayAnimation(currentState->currentNode->animationName, playLoopData);

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
}
