/*
*	Game Engine Project
*	Emre Baris Coskun
*	2018
*/
#include "pch.h"

#include "Renderer.h"

//#include "Engine.h"
//#include "GLPch.h"
//#include "GraphicsManager.h"
//#include "Mesh.h"
//#include "PointLight.h"
//#include "Scene.h"
//#include "WindowManager.h"

//#pragma comment(lib, "glu32.lib") 

Renderer::Renderer(): vertexBufferId_(0), indexBufferId_(0)
{
	//graphicsManager_ = engine->GetGraphicsManager();
}

Renderer::~Renderer() = default;

void Renderer::SetBufferData()
{
//	graphicsManager_->GenerateBuffers(1, &vertexBufferId_);
//	graphicsManager_->GenerateBuffers(1, &indexBufferId_);
//	EXIT_ON_GL_ERROR("ERROR: Could not generate one or more of the buffers.");
//
//	graphicsManager_->BindBuffers(BufferTypes::ArrayBuffer, vertexBufferId_);
//	graphicsManager_->BindBuffers(BufferTypes::ElementArrayBuffer, indexBufferId_);
//	EXIT_ON_GL_ERROR("ERROR: Could not bind one or more of the buffers.");
//
//	const unsigned int vertexSizeInBytes = Scene::mainScene->vertices.size() * sizeof(Scene::mainScene->vertices[0]);
//	graphicsManager_->BufferData(BufferTypes::ArrayBuffer, vertexSizeInBytes, &(Scene::mainScene->vertices[0].position.x), BufferUsages::StaticDraw);
//	EXIT_ON_GL_ERROR("ERROR: Error setting the buffers.");
//
//	size_t vertexDataSizeInBytes = sizeof(VertexData);
//	size_t offset = 0;
//
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexDataSizeInBytes, BUFFER_OFFSET(offset));
//	EXIT_ON_GL_ERROR("ERROR: Error setting the attribute pointers 0.");
//	offset += sizeof(VertexData::position);
//
//	glEnableVertexAttribArray(1);
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertexDataSizeInBytes, BUFFER_OFFSET(offset));
//	EXIT_ON_GL_ERROR("ERROR: Error setting the attribute pointers 1.");
//	offset += sizeof(VertexData::normal);
//
//	glEnableVertexAttribArray(2);
//	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertexDataSizeInBytes, BUFFER_OFFSET(offset));
//	EXIT_ON_GL_ERROR("ERROR: Error setting the attribute pointers 2.");
// 
//	const int meshCount = Scene::mainScene->meshes.size();
//	int faceCount = 0;
//	int faceSizeInBytes = 0;
//	for (int meshIndex = 0; meshIndex < meshCount; meshIndex++)
//	{
//		faceCount += Scene::mainScene->meshes[meshIndex]->faces.size();
//		unsigned int faceSize = faceCount * 3 * sizeof(Scene::mainScene->meshes[meshIndex]->faces[0]->v0);
//		faceSizeInBytes += faceSize;
//	}
//
//	graphicsManager_->BufferData(BufferTypes::ElementArrayBuffer, faceSizeInBytes, &(Scene::mainScene->meshes[0]->faces[0]->v0), BufferUsages::StaticDraw);
//	EXIT_ON_GL_ERROR("ERROR: Could not bind the IBO to the VAO.");
//
//	unsigned int startIndex = 0;
//	for (size_t meshIndex = 0; meshIndex < meshCount; meshIndex++)
//	{
//		unsigned int faceSize = 3 * Scene::mainScene->meshes[meshIndex]->faces.size() * sizeof(Scene::mainScene->meshes[0]->faces[0]->v0);
//
//		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, startIndex, faceSize, &Scene::mainScene->meshes[meshIndex]->faces[0]->v0);
//		EXIT_ON_GL_ERROR("ERROR: Error setting the buffer sub data.");
//		startIndex += faceSize;
//	}
}

void Renderer::Init()
{
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glDepthFunc(GL_LEQUAL);
	//glShadeModel(GL_SMOOTH);
	//EXIT_ON_GL_ERROR("ERROR: Error at Renderer::Init.");

	//SetBufferData();
}

void Renderer::Render()
{
	//glClearColor(Scene::mainScene->bgColor.r, Scene::mainScene->bgColor.g, Scene::mainScene->bgColor.b, 1.f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//
	//size_t meshCount = Scene::mainScene->meshes.size();
	//for (size_t meshIndex = 0; meshIndex < meshCount; meshIndex++)
	//{
	//	MeshBase *currentMesh = Scene::mainScene->meshes[meshIndex];
	//
	//	if(currentMesh->polygonType != POLYGON_TYPE::FILL) 
	//	{
	//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//	}
	//	else 
	//	{
	//		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//	}
	//	
	//	/* SHADER */
	//	glUseProgram(currentMesh->GetShaderProgramId());
	//	EXIT_ON_GL_ERROR("ERROR: Could not use the shader program.");
	//	/*        */

	//	size_t offset = 0;
	//	glDrawElements(GL_TRIANGLES, currentMesh->faces.size() * 3, GL_UNSIGNED_INT, BUFFER_OFFSET(offset));
	//	EXIT_ON_GL_ERROR("ERROR: Could not draw elements.");
	//}

	//glfwSwapBuffers(engine->GetWindowManager()->GetWindow());
}

void Renderer::AddObjectToRenderer(RenderingObject *object)
{
	//objectsToBeRendered_.push_back(object);
}
