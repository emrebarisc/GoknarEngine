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
	animations_ = {};
}

AnimatedSpriteMesh::AnimatedSpriteMesh(Material* material) :
	SpriteMesh(material),
	TimeDependentObject()
{
	animations_ = {};
}

AnimatedSpriteMesh::~AnimatedSpriteMesh()
{
	for (std::unordered_map<std::string, AnimatedSpriteAnimation*>::iterator ite = animations_.begin(); ite != animations_.end(); ite++)
	{
		delete ite->second;
	}
	animations_.clear();
}

void AnimatedSpriteMesh::Init()
{
	if(animations_.count(currentAnimationName_) != 0)
	{
		AnimatedSpriteAnimation* currentAnimation = animations_[currentAnimationName_];
		if (currentAnimation->textureCoordinatesSize_ > 0)
		{
			textureCoordinate_ = currentAnimation->textureCoordinates[0];
		}
	}

	Activate();
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

	if (currentAnimationName_ != name)
	{
		currentAnimationName_ = name;
		Operate();
		elapsedTime_ = 0.f;
	}
}

void AnimatedSpriteMesh::AddAnimation(AnimatedSpriteAnimation* animation)
{
	if (animations_.count(animation->name_) != 0)
	{
		GOKNAR_ERROR("Animation '{}' already exists.", animation->name_);
		delete animation;
		return;
	}
	else
	{
		animations_[animation->name_] = animation;
	}
}

void AnimatedSpriteMesh::Operate()
{
	AnimatedSpriteAnimation* currentAnimation = animations_[currentAnimationName_];
	if (currentAnimation == nullptr)
	{
		return;
	}

	if (currentAnimation->textureCoordinatesIndex_ < currentAnimation->textureCoordinatesSize_ - 1)
	{
		currentAnimation->textureCoordinatesIndex_++;
	}
	else if(currentAnimation->repeat_)
	{
		currentAnimation->textureCoordinatesIndex_ = 0;
	}

	textureCoordinate_ = currentAnimation->textureCoordinates[currentAnimation->textureCoordinatesIndex_];
	UpdateSpriteMeshVertexData();
}
