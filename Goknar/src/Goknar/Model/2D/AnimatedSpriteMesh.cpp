#include "pch.h"

#include "AnimatedSpriteMesh.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Log.h"
#include "Goknar/Scene.h"

AnimatedSpriteMesh::AnimatedSpriteMesh() :
	SpriteMesh(),
	TimeDependentObject()
{
}

AnimatedSpriteMesh::AnimatedSpriteMesh(Material* material) :
	SpriteMesh(material),
	TimeDependentObject()
{
}

void AnimatedSpriteMesh::Init()
{
	if(animations_.count(currentAnimationName_) != 0)
	{
		AnimatedSpriteAnimation* currentAnimation = animations_[currentAnimationName_];
		if (currentAnimation->textureCoordinatesSize > 0)
		{
			textureCoordinate_ = currentAnimation->textureCoordinates[0];
		}
	}

	SpriteMesh::Init();
}

void AnimatedSpriteMesh::PlayAnimation(const std::string& name)
{
	if (!isInitialized_)
	{
		GOKNAR_ERROR("Animation cannot be played at constructor or before initialization.", name);
		return;
	}

	if (animations_.count(name) == 0)
	{
		GOKNAR_ERROR("Animation '{}' is not found.", name);
		return;
	}

	currentAnimationName_ = name;
	Operate();
	animationElapsedTime_ = 0.f;
}

void AnimatedSpriteMesh::AddAnimation(AnimatedSpriteAnimation* animation)
{
	if (animations_.count(animation->name) != 0)
	{
		GOKNAR_ERROR("Animation '{}' already exists.", animation->name);
		return;
	}
	else
	{
		animations_[animation->name] = animation;
	}
}

void AnimatedSpriteMesh::Operate()
{
	AnimatedSpriteAnimation* currentAnimation = animations_[currentAnimationName_];
	if (currentAnimation == nullptr)
	{
		return;
	}

	if (currentAnimation->textureCoordinatesIndex < currentAnimation->textureCoordinatesSize - 1)
	{
		currentAnimation->textureCoordinatesIndex++;
	}
	else
	{
		currentAnimation->textureCoordinatesIndex = 0;
	}

	textureCoordinate_ = currentAnimation->textureCoordinates[currentAnimation->textureCoordinatesIndex];
	UpdateSpriteMeshVertexData();
}
