#ifndef __SPRITEMESH_H__
#define __SPRITEMESH_H__

#include "Goknar/Model/DynamicMesh.h"

#include "Goknar/Geometry/Rect.h"

class Material;

class GOKNAR_API SpriteMesh : public DynamicMesh
{
public:
	SpriteMesh();
	SpriteMesh(Material* material);

	~SpriteMesh()
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

protected:
	void UpdateSpriteMeshVertexData();

	Rect textureCoordinate_;

	Material* material_;

	float width_;
	float height_;

private:
	void NormalizeTexturePosition();
};

#endif