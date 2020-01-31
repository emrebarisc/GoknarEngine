#ifndef __SPRITEMESH_H__
#define __SPRITEMESH_H__

#include "Mesh.h"

class Texture;

class GOKNAR_API SpriteMesh : public Mesh
{
public:
	SpriteMesh();
	SpriteMesh(Texture* texture);

	~SpriteMesh()
	{

	}

	void Init() override;

private:
	Texture* texture_;
};

#endif