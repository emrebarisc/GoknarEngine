#ifndef __ANIMATEDSPRITEMESH_H__
#define __ANIMATEDSPRITEMESH_H__

#include "SpriteMesh.h"
#include "Goknar/TimeDependentObject.h"

class Component;
class Material;

class GOKNAR_API AnimatedSpriteMesh : public SpriteMesh, public TimeDependentObject
{
public:
	AnimatedSpriteMesh();
	AnimatedSpriteMesh(Material* material);

	~AnimatedSpriteMesh()
	{

	}

	void Init() override;

	void AddTextureCoordinate(const Rect& textureCoordinate)
	{
		textureCoordinates_.push_back(textureCoordinate);
		textureCoordinatesSize_++;
	}

	void ResetTextureCoordinateAt(int index, const Rect& textureCoordinate)
	{
		textureCoordinates_[index] = textureCoordinate;
	}

	void RemoveTextureCoordinateAt(int index)
	{
		textureCoordinates_.erase(textureCoordinates_.begin() + index);
		textureCoordinatesSize_++;
	}

private:
	void Operate() override;

	int textureCoordinatesIndex_;
	int textureCoordinatesSize_;

	// TODO: Change vector with a "better" container.
	std::vector<Rect> textureCoordinates_;
};

#endif