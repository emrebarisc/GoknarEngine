#ifndef __RENDERTOTEXTUREOBJECT_H__
#define __RENDERTOTEXTUREOBJECT_H__

#include "Goknar/ObjectBase.h"

class StaticMeshComponent;

class RenderToTextureObject : public ObjectBase
{
public:
	RenderToTextureObject();

private:

	StaticMeshComponent* renderToTextureMeshComponent;
};

#endif