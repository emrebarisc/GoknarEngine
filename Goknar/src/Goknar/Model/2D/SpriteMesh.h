#ifndef __SPRITEMESH_H__
#define __SPRITEMESH_H__

#include "Goknar/Model/Mesh.h"

class Material;

class GOKNAR_API SpriteMesh : public Mesh
{
public:
	SpriteMesh();
	SpriteMesh(Material* material);

	~SpriteMesh()
	{

	}

	void Init() override;

	void SetSize(int width, int height)
	{
		width_ = width;
		height_ = height;
	}

private:
	Material* material_;

	int width_;
	int height_;
};

#endif