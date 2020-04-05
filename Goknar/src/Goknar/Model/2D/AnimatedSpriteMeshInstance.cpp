#include "pch.h"

#include "AnimatedSpriteMeshInstance.h"

#include "Goknar/Model/2D/AnimatedspriteMesh.h"
#include "Goknar/Engine.h"
#include "Goknar/Renderer/Renderer.h"

AnimatedSpriteMeshInstance::AnimatedSpriteMeshInstance(Component* parentComponent) : 
	DynamicMeshInstance(parentComponent),
	animatedspriteMesh_(nullptr)
{

}

void AnimatedSpriteMeshInstance::SetAnimatedSpriteMesh(AnimatedSpriteMesh* animatedspriteMesh)
{
	animatedspriteMesh_ = animatedspriteMesh;
	DynamicMeshInstance::SetMesh(animatedspriteMesh);
}
