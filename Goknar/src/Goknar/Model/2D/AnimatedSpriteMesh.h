#ifndef __ANIMATEDSPRITEMESH_H__
#define __ANIMATEDSPRITEMESH_H__

#include <unordered_map>

#include "SpriteMesh.h"
#include "Goknar/TimeDependentObject.h"

class Component;
class Material;

class AnimatedSpriteAnimation
{
	friend class AnimatedSpriteMesh;
public:
	AnimatedSpriteAnimation() = delete;
	AnimatedSpriteAnimation(const std::string& animationName) :
		name_(animationName),
		textureCoordinatesIndex_(0),
		textureCoordinatesSize_(0),
		repeat_(true)
	{

	}

	void AddTextureCoordinate(const Rect& textureCoordinate)
	{
		textureCoordinates.push_back(textureCoordinate);
		textureCoordinatesSize_++;
	}

	void ResetTextureCoordinateAt(int index, const Rect& textureCoordinate)
	{
		textureCoordinates[index] = textureCoordinate;
	}

	void RemoveTextureCoordinateAt(int index)
	{
		textureCoordinates.erase(textureCoordinates.begin() + index);
		textureCoordinatesSize_++;
	}

	void SetRepeat(bool repeat)
	{
		repeat_ = repeat;
	}

private:
	// TODO: Change vector with a "better" container.
	std::vector<Rect> textureCoordinates;

	std::string name_;

	int textureCoordinatesIndex_;
	int textureCoordinatesSize_;

	bool repeat_;
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