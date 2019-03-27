/*
*	Game Engine Project
*	Emre Baris Coskun
*	2018
*/

#include "GraphicsManager.h"

#include "GLFW/glfw3.h"

#include <iostream>
#include <cassert>

GraphicsManager::GraphicsManager() = default;

GraphicsManager::~GraphicsManager()
{
	glfwTerminate();
}

void GraphicsManager::Init()
{
	//if (glewInit() != GLEW_OK) {
	//	std::cerr << "Failed to initialize GLEW\n";
	//	glfwTerminate();
	//	exit(1);
	//}*/

	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);
	//// For 2D:
	////glShadeModel(GL_FLAT);
	//glDisable(GL_CULL_FACE);
}

void GraphicsManager::GenerateBuffers(const int size, gm_uint *bufferId)
{
	//glGenBuffers(size, bufferId);
}

void GraphicsManager::BindBuffers(BufferTypes bufferType, gm_uint bufferId)
{
	//GLenum glBufferType = 0;

	//switch (bufferType)
	//{
	//case(BufferTypes::ArrayBuffer):
	//	glBufferType = GL_ARRAY_BUFFER;
	//	break;
	//case BufferTypes::ElementArrayBuffer:
	//	glBufferType = GL_ELEMENT_ARRAY_BUFFER;
	//	break;
	//case BufferTypes::TextureBuffer: 
	//	glBufferType = GL_TEXTURE_BUFFER;
	//	break;
	//default:
	//	assert("Invalid buffer!");
	//}

	//glBindBuffer(glBufferType, bufferId);
}

void GraphicsManager::GenerateVertexArrays(const int size, gm_uint *arrayId)
{
	//glGenVertexArrays(size, arrayId);
}

void GraphicsManager::BindVertexArray(const int arrayId)
{
	//glBindVertexArray(arrayId);
}

void GraphicsManager::BufferData(BufferTypes bufferType, int size, const void* bufferPointer, BufferUsages usage)
{
	//GLenum glBufferType = 0;

	//switch (bufferType)
	//{
	//case BufferTypes::ArrayBuffer:
	//	glBufferType = GL_ARRAY_BUFFER;
	//	break;
	//case BufferTypes::ElementArrayBuffer:
	//	glBufferType = GL_ELEMENT_ARRAY_BUFFER;
	//	break;
	//case BufferTypes::TextureBuffer:
	//	glBufferType = GL_TEXTURE_BUFFER;
	//	break;
	//default:
	//	assert("Invalid buffer!");
	//}

	//glBufferData(glBufferType, size, bufferPointer, usage == BufferUsages::StaticDraw ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
}

void GraphicsManager::DeleteBuffers(const int size, gm_uint *bufferId)
{
	//glDeleteBuffers(size, bufferId);
}

void GraphicsManager::DeleteVertexArrays(const int size, gm_uint *arrayId)
{
	//glDeleteVertexArrays(size, arrayId);
}