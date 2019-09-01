#include "pch.h"

#include "Renderer.h"

#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/Log.h"
#include "Goknar/Mesh.h"
#include <gl\GLU.h>

#include "Goknar/Shader.h"
#include "Goknar/Managers/IOManager.h"

#include <Windows.h>

Renderer::Renderer(): vertexBufferId_(0), indexBufferId_(0), totalVertexSize_(0), totalFaceSize_(0)
{
}

Renderer::~Renderer()
{
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glDeleteBuffers(1, &vertexBufferId_);
	glDeleteBuffers(1, &indexBufferId_);
}

void Renderer::SetBufferData()
{
	/*
		Vertex buffer
	*/
	unsigned long long sizeOfVertexData = sizeof(VertexData);

	glGenBuffers(1, &vertexBufferId_);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId_);
	glBufferData(GL_ARRAY_BUFFER, totalVertexSize_ * sizeOfVertexData, nullptr, GL_STATIC_DRAW);

	/*
		Index buffer
	*/
	glGenBuffers(1, &indexBufferId_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalFaceSize_ * sizeof(Face), nullptr, GL_STATIC_DRAW);

	/*
		Buffer Sub-Data
	*/
	int vertexStartIndex = 0;
	int faceStartIndex = 0;
	for (const Mesh* mesh : objectsToBeRendered_)
	{
		const VertexArray* vertexArrayPtr = mesh->GetVerticesPointer();
		int vertexSizeInBytes = vertexArrayPtr->size() * sizeof(vertexArrayPtr->at(0));
		glBufferSubData(GL_ARRAY_BUFFER, vertexStartIndex, vertexSizeInBytes, &vertexArrayPtr->at(0));

		const FaceArray* faceArrayPtr = mesh->GetFacesPointer();
		int faceSizeInBytes = faceArrayPtr->size() * sizeof(faceArrayPtr->at(0));
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, faceStartIndex, faceSizeInBytes, &faceArrayPtr->at(0));

		vertexStartIndex += vertexSizeInBytes;
		faceStartIndex += faceSizeInBytes;
	}

	// Vertex position
	int offset = 0;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeOfVertexData, (void*)offset);

	// Vertex normal
	offset += sizeof(VertexData::position);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeOfVertexData, (void*)offset);

	// Vertex UV
	offset += sizeof(VertexData::normal);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeOfVertexData, (void*)offset);
}

void Renderer::Init()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	
	for (Mesh* mesh : objectsToBeRendered_)
	{
		totalVertexSize_ += mesh->GetVerticesPointer()->size();
		totalFaceSize_ += mesh->GetFacesPointer()->size();

		mesh->Init();
	}

	SetBufferData();
}

void Renderer::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	const Colori& sceneBackgroundColor = engine->GetApplication()->GetMainScene()->GetBackgroundColor();
	glClearColor(sceneBackgroundColor.r, sceneBackgroundColor.g, sceneBackgroundColor.b, 1.f);
	
	int vertexStartingIndex = 0;
	int baseVertex = 0;
	for (const Mesh* mesh : objectsToBeRendered_)
	{
		mesh->GetShader()->Use();
		mesh->Render();

		int facePointCount = mesh->GetFaceCount() * 3;
		glDrawElementsBaseVertex(GL_TRIANGLES, facePointCount, GL_UNSIGNED_INT, (void*)vertexStartingIndex, baseVertex);
		vertexStartingIndex += facePointCount * sizeof(Face::vertexIndices[0]);
		baseVertex += mesh->GetVertexCount();
	}
}

void Renderer::AddObjectToRenderer(Mesh*object)
{
	objectsToBeRendered_.push_back(object);
}
