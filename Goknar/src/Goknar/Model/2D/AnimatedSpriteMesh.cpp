#include "pch.h"

#include "AnimatedSpriteMesh.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"

AnimatedSpriteMesh::AnimatedSpriteMesh() : SpriteMesh(), TimeDependentObject()
{
}

AnimatedSpriteMesh::AnimatedSpriteMesh(Material* material) : SpriteMesh(material)
{
}

void AnimatedSpriteMesh::Init()
{
	SpriteMesh::Init();
}

void AnimatedSpriteMesh::Operate()
{
	std::cout << "Operate" << std::endl;
}
