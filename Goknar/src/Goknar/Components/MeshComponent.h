#ifndef __MESHCOMPONENT_H__
#define __MESHCOMPONENT_H__

#include "Component.h"

#include "Goknar/Core.h"

class Mesh;
class ObjectBase;

class GOKNAR_API MeshComponent : public Component
{
public:
	MeshComponent() = delete;
	MeshComponent(const ObjectBase* parent);
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