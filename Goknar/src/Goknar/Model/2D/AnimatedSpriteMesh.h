#ifndef __ANIMATEDSPRITEMESH_H__
#define __ANIMATEDSPRITEMESH_H__

#include <unordered_map>

#include "SpriteMesh.h"
#include "Goknar/TimeDependentObject.h"

class Component;
class Material;

class AnimatedSpriteAnimation
{
public:
	AnimatedSpriteAnimation() = delete;
	AnimatedSpriteAnimation(const std::string& animationName) : 
		name(animationName),
		textureCoordinatesIndex(0),
		textureCoordinatesSize(0)
	{

	}

	void AddTextureCoordinate(const Rect& textureCoordinate)
	{
		textureCoordinates.push_back(textureCoordinate);
		textureCoordinatesSize++;
	}

	void ResetTextureCoordinateAt(int index, const Rect& textureCoordinate)
	{
		textureCoordinates[index] = textureCoordinate;
	}

	void RemoveTextureCoordinateAt(int index)
	{
		textureCoordinates.erase(textureCoordinates.begin() + index);
		textureCoordinatesSize++;
	}

	// TODO: Change vector with a "better" container.
	std::vector<Rect> textureCoordinates;

	std::string name;

	int textureCoordinatesIndex;
	int textureCoordinatesSize;
};

class GOKNAR_API AnimatedSpriteMesh : public SpriteMesh, public TimeDependentObject
{
public:
	AnimatedSpriteMesh();
	AnimatedSpriteMesh(Material* material);

	virtual ~AnimatedSpriteMesh();

	void Init() override;

	void PlayAnimation(const std::string& name);
	void AddAnimation(AnimatedSpriteAnimation* animation);

	const std::unordered_map<std::string, AnimatedSpriteAnimation*>& GetAnimationsHashMap() const
	{
		return animations_;
	}

private:
	void Operate() override;

	std::string currentAnimationName_;
	std::unordered_map<std::string, AnimatedSpriteAnimation*> animations_;
};

#endif