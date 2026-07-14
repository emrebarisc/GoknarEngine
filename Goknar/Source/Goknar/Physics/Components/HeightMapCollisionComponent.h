#ifndef __HEIGHTMAPCOLLISIONCOMPONENT_H__
#define __HEIGHTMAPCOLLISIONCOMPONENT_H__

#include "CollisionComponent.h"

class Image;

// For height map(terrain) collisions
class GOKNAR_API HeightMapCollisionComponent : public CollisionComponent
{
public:
	HeightMapCollisionComponent(Component* parent);
	HeightMapCollisionComponent(ObjectBase* parentObjectBase);
	~HeightMapCollisionComponent();

	virtual void PreInit() override;
	virtual void Init() override;
	virtual void PostInit() override;
	
	virtual void BeginGame() override;
	virtual void TickComponent(float deltaTime) override;
	Component* Clone() const override;

	void SetHeightMapImage(const Image* heightMapImage)
	{
		heightMapImage_ = heightMapImage;
	}

	const Image* GetHeightMapImage() const
	{
		return heightMapImage_;
	}

	void SetHeightStickWidth(int heightStickWidth)
	{
		heightStickWidth_ = heightStickWidth;
	}

	int GetHeightStickWidth() const
	{
		return heightStickWidth_;
	}

	void SetHeightStickLength(int heightStickLength)
	{
		heightStickLength_ = heightStickLength;
	}

	int GetHeightStickLength() const
	{
		return heightStickLength_;
	}

	void SetMinHeight(float minHeight)
	{
		minHeight_ = minHeight;
	}

	float GetMinHeight() const
	{
		return minHeight_;
	}

	void SetMaxHeight(float maxHeight)
	{
		maxHeight_ = maxHeight;
	}

	float GetMaxHeight() const
	{
		return maxHeight_;
	}

	void SetWidth(float width)
	{
		width_ = width;
	}

	float GetWidth() const
	{
		return width_;
	}

	void SetLength(float length)
	{
		length_ = length;
	}

	float GetLength() const
	{
		return length_;
	}

	void SetHeightScale(float heightScale)
	{
		heightScale_ = heightScale;
	}

	float GetHeightScale() const
	{
		return heightScale_;
	}

protected:
	const Image* heightMapImage_{ nullptr };

	int heightStickWidth_{  0 };
	int heightStickLength_{  0 };

	float minHeight_{  0.0f };
	float maxHeight_{  0.0f };

	float width_{  1.0f };
	float length_{  1.0f };
	float heightScale_{  1.0f };
private:
};

#endif