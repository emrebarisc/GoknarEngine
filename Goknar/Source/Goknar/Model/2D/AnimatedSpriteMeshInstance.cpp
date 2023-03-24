#include "pch.h"

#include "AnimatedSpriteMeshInstance.h"

#include "Goknar/Model/2D/AnimatedSpriteMesh.h"
#include "Goknar/Engine.h"
#include "Goknar/Renderer/Renderer.h"

AnimatedSpriteMeshInstance::AnimatedSpriteMeshInstance(RenderComponent* parentComponent) : 
	DynamicMeshInstance(parentComponent),
	animatedspriteMesh_(nullptr)
{

}

void AnimatedSpriteMeshInstance::SetAnimatedSpriteMesh(AnimatedSpriteMesh* animatedspriteMesh)
{
	animatedspriteMesh_ = animatedspriteMesh;
	DynamicMeshInstance::SetMesh(animatedspriteMesh);
}
