#ifndef __SPRITEMESH_H__
#define __SPRITEMESH_H__

#include "Goknar/Model/DynamicMesh.h"

#include "Goknar/Geometry/Rect.h"

class Material;

enum class GOKNAR_API SPRITE_PIVOT_POINT
{
	TOP_LEFT = 0,
	TOP_MIDDLE,
	TOP_RIGHT,
	MIDDLE_LEFT,
	MIDDLE_MIDDLE,
	MIDDLE_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_MIDDLE,
	BOTTOM_RIGHT
};

class GOKNAR_API SpriteMesh : public DynamicMesh
{
public:
	SpriteMesh();
	SpriteMesh(Material* material);

	virtual ~SpriteMesh()
	{

	}

	void Init() override;

	void SetSize(float width, float height)
	{
		width_ = width;
		height_ = height;
	}

	void SetTextureCoordinate(const Rect& textureCoordinate);
	const Rect& GetTextureCoordinate() const
	{
		return textureCoordinate_;
	}

	void SetPivotPoint(SPRITE_PIVOT_POINT pivotPoint)
	{
		pivotPoint_ = pivotPoint;
	}

	SPRITE_PIVOT_POINT GetPivotPoint() const
	{
		return pivotPoint_;
	}

protected:
	void UpdateSpriteMeshVertexData();

	Rect textureCoordinate_;

	Material* material_;

	float width_;
	float height_;

	SPRITE_PIVOT_POINT pivotPoint_;

private:
	void NormalizeTexturePosition();
};

#endif