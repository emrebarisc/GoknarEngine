#ifndef __MESHCOMPONENT_H__
#define __MESHCOMPONENT_H__

#include "Goknar/Core.h"

class Mesh;

class GOKNAR_API MeshComponent
{
public:
	MeshComponent();
	~MeshComponent();

	void SetMesh(Mesh* mesh);
	Mesh* GetMesh()
	{
		return mesh_;
	}

protected:

private:
	Mesh* mesh_;

};
#endif