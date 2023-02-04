#include "GameController.h"

// Engine includes
#include "Goknar/Engine.h"
#include "Goknar/Managers/InputManager.h"
#include "Goknar/Managers/WindowManager.h"

#include "Goknar/Material.h"
#include "Goknar/Components/SkeletalMeshComponent.h"
#include "Goknar/Renderer/Shader.h"
#include "Goknar/Model/SkeletalMesh.h"
#include "Goknar/Model/SkeletalMeshInstance.h"

#include "Mutant.h"

#define MAX_BONE_INDEX 37
#define MAX_ANIMATION_INDEX 19

GameController::GameController() : 
	Controller(),
	mutant(nullptr),
	useNormalTexture(true)
{
	currentBoneIndex = 0;
	animationIndex = 0;
}

void GameController::SetupInputs()
{
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::UP, INPUT_ACTION::G_PRESS, std::bind(&GameController::IncreaseCurrentBoneIndex, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::DOWN, INPUT_ACTION::G_PRESS, std::bind(&GameController::DecreaseCurrentBoneIndex, this));

	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::RIGHT, INPUT_ACTION::G_PRESS, std::bind(&GameController::IncreaseAnimationIndex, this));
	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::LEFT, INPUT_ACTION::G_PRESS, std::bind(&GameController::DecreaseAnimationIndex, this));

	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_PRESS, std::bind(&GameController::ToggleNormalTexture, this));

	engine->GetInputManager()->AddKeyboardInputDelegate(KEY_MAP::F, INPUT_ACTION::G_PRESS, std::bind(&GameController::ToggleFullscreen, this));
}

void GameController::IncreaseCurrentBoneIndex()
{
	currentBoneIndex = (currentBoneIndex + 1) % MAX_BONE_INDEX;
	GOKNAR_INFO("Current bone is {}", mutant->GetSkeletalMesh()->GetBone(currentBoneIndex)->name);
	mutant->GetSkeletalMesh()->GetMaterial()->GetShader()->SetInt("currentBoneIndex", currentBoneIndex);
}

void GameController::DecreaseCurrentBoneIndex()
{
	--currentBoneIndex;
	if (currentBoneIndex < 0)
	{
		currentBoneIndex += MAX_BONE_INDEX;
	}
	GOKNAR_INFO("Current bone is {}", mutant->GetSkeletalMesh()->GetBone(currentBoneIndex)->name);
	mutant->GetSkeletalMesh()->GetMaterial()->GetShader()->SetInt("currentBoneIndex", currentBoneIndex);
}

void GameController::ToggleFullscreen()
{
	engine->GetWindowManager()->ToggleFullscreen();
}

void GameController::ToggleNormalTexture()
{
	useNormalTexture = !useNormalTexture;
	mutant->GetSkeletalMesh()->GetMaterial()->GetShader()->SetInt("useNormalTexture", useNormalTexture);
}

void GameController::IncreaseAnimationIndex()
{
	animationIndex = (animationIndex + 1) % MAX_ANIMATION_INDEX;
	SetAnimation();
}

void GameController::DecreaseAnimationIndex()
{
	--animationIndex;
	if (animationIndex < 0)
	{
		animationIndex += MAX_ANIMATION_INDEX;
	}
	SetAnimation();
}

void GameController::SetAnimation()
{
	SkeletalMeshInstance* skeletalMeshInstance = dynamic_cast<SkeletalMeshInstance*>(mutant->GetSkeletalMeshComponent()->GetMeshInstance());

	if (skeletalMeshInstance)
	{
		skeletalMeshInstance->PlayAnimation("Armature|Armature|mixamo.com|Layer0");
		return;

		switch (animationIndex)
		{
		case 0:
			skeletalMeshInstance->PlayAnimation("Armature|Attack");
			break;
		case 1:
			skeletalMeshInstance->PlayAnimation("Armature|BreathingIdle");
			break;
		case 2:
			skeletalMeshInstance->PlayAnimation("Armature|Death");
			break;
		case 3:
			skeletalMeshInstance->PlayAnimation("Armature|Idle1");
			break;
		case 4:
			skeletalMeshInstance->PlayAnimation("Armature|Idle2");
			break;
		case 5:
			skeletalMeshInstance->PlayAnimation("Armature|Idle3");
			break;
		case 6:
			skeletalMeshInstance->PlayAnimation("Armature|WalkInPlace");
			break;
		case 7:
			skeletalMeshInstance->PlayAnimation("Armature|Jump");
			break;
		case 8:
			skeletalMeshInstance->PlayAnimation("Armature|JumpAttack");
			break;
		case 9:
			skeletalMeshInstance->PlayAnimation("Armature|JumpInPlace");
			break;
		case 10:
			skeletalMeshInstance->PlayAnimation("Armature|JumpInPlaceStart");
			break;
		case 11:
			skeletalMeshInstance->PlayAnimation("Armature|Punch");
			break;
		case 12:
			skeletalMeshInstance->PlayAnimation("Armature|Roar");
			break;
		case 13:
			skeletalMeshInstance->PlayAnimation("Armature|Run");
			break;
		case 14:
			skeletalMeshInstance->PlayAnimation("Armature|TurnLeft45Degrees");
			break;
		case 15:
			skeletalMeshInstance->PlayAnimation("Armature|TurnLeftInPlace");
			break;
		case 16:
			skeletalMeshInstance->PlayAnimation("Armature|TurnRight45Degrees");
			break;
		case 17:
			skeletalMeshInstance->PlayAnimation("Armature|TurnRight90Degrees");
			break;
		case 18:
			skeletalMeshInstance->PlayAnimation("Armature|TurnRightInPlace");
			break;
		default:
			break;
		}
	}

}