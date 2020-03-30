#include "pch.h"

#include "AnimatedSpriteMesh.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"

AnimatedSpriteMesh::AnimatedSpriteMesh() : 
	SpriteMesh(), 
	TimeDependentObject(),
	textureCoordinatesIndex_(0),
	textureCoordinatesSize_(0)
{
}

AnimatedSpriteMesh::AnimatedSpriteMesh(Material* material) : 
	SpriteMesh(material),
	TimeDependentObject(),
	textureCoordinatesIndex_(0),
	textureCoordinatesSize_(0)
{
}

void AnimatedSpriteMesh::Init()
{
	if (textureCoordinatesSize_ > 0)
	{
		texturePosition_ = textureCoordinates_[0];
	}
	SpriteMesh::Init();
}

void AnimatedSpriteMesh::Operate()
{
	if (textureCoordinatesIndex_ < textureCoordinatesSize_ - 1)
	{
		textureCoordinatesIndex_++;
	}
	else
	{
		textureCoordinatesIndex_ = 0;
	}

	texturePosition_ = textureCoordinates_[textureCoordinatesIndex_];
	UpdateSpriteMeshVertexData();
}
