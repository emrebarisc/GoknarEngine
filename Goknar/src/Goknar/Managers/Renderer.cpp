#include "pch.h"

#include "Renderer.h"

#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/Log.h"
#include "Goknar/Mesh.h"
#include <gl\GLU.h>

#include "Goknar/Shader.h"
#include "IOManager.h"

#include <Windows.h>

Renderer::Renderer(): vertexBufferId_(0), indexBufferId_(0)
{
}

Renderer::~Renderer() = default;

void Renderer::SetBufferData()
{
}

void Renderer::Init()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	
	for (Mesh* mesh : objectsToBeRendered_)
	{
		mesh->Init();
	}

	//glGenVertexArrays(1, &vertexArrayId_);
	//glBindVertexArray(vertexArrayId_);

	//glGenBuffers(1, &vertexBufferId_);
	//glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId_);

	//float vertices[9] =
	//{
	//	-0.5f, -0.5f, 0.f,
	//	0.5f, -0.5f, 0.f,
	//	0.f, 0.5f, 0.f,
	//};

	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

	//glGenBuffers(1, &indexBufferId_);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId_);

	//unsigned int indices[3] = { 0, 1, 2 };
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//// TODO: REMOVE SHADER OPERATION HERE
	//const char* vertexBuffer =
	//	R"(
	//		#version 330 core

	//		layout(location = 0) in vec3 position;
	//		
	//		void main()
	//		{
	//			gl_Position = vec4(position, 1.0);
	//		}
	//	)";

	//const char* fragmentBuffer =
	//	R"(
	//		#version 330

	//		out vec4 color;

	//		void main() 
	//		{
	//		  color = vec4(0.2, 0.8, 0.2, 1.0);
	//		}
	//	)";

	///*char* vertexBuffer = nullptr;
	//if (IOManager::GetFileRawText("C:/Users/emreb/Desktop/DefaultShadersDefaultVertexShader.glsl", vertexBuffer))
	//{

	//}

	//char* fragmentBuffer = nullptr;
	//if (IOManager::GetFileRawText("C:/Users/emreb/Desktop/DefaultShadersDefaultFragmentShader.glsl", fragmentBuffer))
	//{ 

	//}*/
	//
	//shader_ = new Shader(vertexBuffer, fragmentBuffer);
}

void Renderer::Render()
{
	const Colori& sceneBackgroundColor = Scene::mainScene->GetBackgroundColor();
	glClearColor(GLfloat(sceneBackgroundColor.r), GLfloat(sceneBackgroundColor.g), GLfloat(sceneBackgroundColor.b), 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// TODO Render scene
	glBindVertexArray(vertexArrayId_);

	for (Mesh* mesh : objectsToBeRendered_)
	{
		mesh->Render();
	}

	/*glUseProgram(shader_->GetProgramId());

	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);*/
}

void Renderer::AddObjectToRenderer(Mesh*object)
{
	objectsToBeRendered_.push_back(object);
}
 