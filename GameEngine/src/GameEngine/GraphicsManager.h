/*
*	Game Engine Project
*	Emre Baris Coskun
*	2018
*/

#ifndef __GRAPHICSMANAGER_H__
#define __GRAPHICSMANAGER_H__

#include "Core.h"

#include "GLFW/glfw3.h"

typedef GLuint gm_uint;

/*
 *	In GraphicsManager class 
 *	main purpose is to be able to implement 
 *	multi graphics libraries easily.
 */

enum class BufferTypes
{
	ArrayBuffer,
	ElementArrayBuffer,
	TextureBuffer
};

enum class BufferUsages
{
	StaticDraw,
	DynamicDraw
};

enum class GraphicsEnums
{
	CullFace,
	DepthTest,
	Lighting,
	Never,
	Less,
	Equal,
	Lequal,
	Greater,
	NotEqual,
	Gequal,
	Always
};

class GAMEENGINE_API GraphicsManager
{
public:
	GraphicsManager();
	~GraphicsManager();
	
	void Init();

	// General functions
	
	// Buffer related functions
	void GenerateBuffers(int size, gm_uint *bufferId);
	void BindBuffers(BufferTypes bufferType, gm_uint bufferId);
	void GenerateVertexArrays(int size, gm_uint *arrayId);
	void BindVertexArray(int arrayId);
	void BufferData(BufferTypes bufferType, int size, const void *bufferPointer, BufferUsages usage);
	void DeleteBuffers(int size, gm_uint *bufferId);
	void DeleteVertexArrays(int size, gm_uint *arrayId);

	// Render related functions
private:
};

#endif
