#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Goknar/Core.h"

#include "glad/glad.h"

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

	GLuint vertexArrayId_;
	GLuint vertexBufferId_;
	GLuint indexBufferId_;


	// TODO REMOVE SHADER OPERATIONS
	class Shader* shader_;
};

#endif
