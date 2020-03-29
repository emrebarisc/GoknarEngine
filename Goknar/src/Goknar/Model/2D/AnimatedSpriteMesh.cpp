#include "pch.h"

#include "AnimatedSpriteMesh.h"

AnimatedSpriteMesh::AnimatedSpriteMesh() : SpriteMesh()
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
}
