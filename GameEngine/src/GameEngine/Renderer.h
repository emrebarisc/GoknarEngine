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

class GAMEENGINE_API RenderingObject;

class GAMEENGINE_API Renderer
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

	gm_uint vertexBufferId_;
	gm_uint indexBufferId_;
};

#endif
