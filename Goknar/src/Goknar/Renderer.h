/*
*	Game Engine Project
*	Emre Baris Coskun
*	2018
*/

#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Core.h"
#include "GraphicsManager.h"

#include <vector>

class GOKNAR_API RenderingObject;

class GOKNAR_API Renderer
{
public:
	Renderer();
	~Renderer();

	void SetBufferData();
	void Init();

	void Render();

	void AddObjectToRenderer(RenderingObject *object);

private:
	std::vector<RenderingObject *> objectsToBeRendered_;

	GraphicsManager *graphicsManager_;

	gmUInt vertexBufferId_;
	gmUInt indexBufferId_;
};

#endif
