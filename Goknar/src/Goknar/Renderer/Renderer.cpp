#include "pch.h"

#include "Renderer.h"

#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/Log.h"
#include "Goknar/Material.h"
#include "Goknar/Model/Mesh.h"

#include "Goknar/Managers/IOManager.h"
#include "Goknar/Renderer/Shader.h"

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
	for (const Mesh* opaqueMesh : opaqueObjectsToBeRendered_)
	{
		const VertexArray* vertexArrayPtr = opaqueMesh->GetVerticesPointer();
		int vertexSizeInBytes = vertexArrayPtr->size() * sizeof(vertexArrayPtr->at(0));
		glBufferSubData(GL_ARRAY_BUFFER, vertexStartIndex, vertexSizeInBytes, &vertexArrayPtr->at(0));

		const FaceArray* faceArrayPtr = opaqueMesh->GetFacesPointer();
		int faceSizeInBytes = faceArrayPtr->size() * sizeof(faceArrayPtr->at(0));
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, faceStartIndex, faceSizeInBytes, &faceArrayPtr->at(0));

		vertexStartIndex += vertexSizeInBytes;
		faceStartIndex += faceSizeInBytes;
	}

	for (const Mesh* maskedMesh : maskedObjectsToBeRendered_)
	{
		const VertexArray* vertexArrayPtr = maskedMesh->GetVerticesPointer();
		int vertexSizeInBytes = vertexArrayPtr->size() * sizeof(vertexArrayPtr->at(0));
		glBufferSubData(GL_ARRAY_BUFFER, vertexStartIndex, vertexSizeInBytes, &vertexArrayPtr->at(0));

		const FaceArray* faceArrayPtr = maskedMesh->GetFacesPointer();
		int faceSizeInBytes = faceArrayPtr->size() * sizeof(faceArrayPtr->at(0));
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, faceStartIndex, faceSizeInBytes, &faceArrayPtr->at(0));

		vertexStartIndex += vertexSizeInBytes;
		faceStartIndex += faceSizeInBytes;
	}

	for (const Mesh* translucentMesh : translucentObjectsToBeRendered_)
	{
		const VertexArray* vertexArrayPtr = translucentMesh->GetVerticesPointer();
		int vertexSizeInBytes = vertexArrayPtr->size() * sizeof(vertexArrayPtr->at(0));
		glBufferSubData(GL_ARRAY_BUFFER, vertexStartIndex, vertexSizeInBytes, &vertexArrayPtr->at(0));

		const FaceArray* faceArrayPtr = translucentMesh->GetFacesPointer();
		int faceSizeInBytes = faceArrayPtr->size() * sizeof(faceArrayPtr->at(0));
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, faceStartIndex, faceSizeInBytes, &faceArrayPtr->at(0));

		vertexStartIndex += vertexSizeInBytes;
		faceStartIndex += faceSizeInBytes;
	}

	// Vertex position
	int offset = 0;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeOfVertexData, (void*)offset);
	glEnableVertexAttribArray(0);

	// Vertex normal
	offset += sizeof(VertexData::position);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeOfVertexData, (void*)offset);
	glEnableVertexAttribArray(1);

	// Vertex UV
	offset += sizeof(VertexData::normal);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeOfVertexData, (void*)offset);
}

void Renderer::Init()
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	for (Mesh* opaqueMesh : opaqueObjectsToBeRendered_)
	{
		totalVertexSize_ += opaqueMesh->GetVerticesPointer()->size();
		totalFaceSize_ += opaqueMesh->GetFacesPointer()->size();
	}
	for (Mesh* maskedMesh : maskedObjectsToBeRendered_)
	{
		totalVertexSize_ += maskedMesh->GetVerticesPointer()->size();
		totalFaceSize_ += maskedMesh->GetFacesPointer()->size();
	}
	for (Mesh* translucentMesh : translucentObjectsToBeRendered_)
	{
		totalVertexSize_ += translucentMesh->GetVerticesPointer()->size();
		totalFaceSize_ += translucentMesh->GetFacesPointer()->size();
	}

	SetBufferData();
}

void Renderer::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	const Colorf& sceneBackgroundColor = engine->GetApplication()->GetMainScene()->GetBackgroundColor();
	glClearColor(sceneBackgroundColor.r, sceneBackgroundColor.g, sceneBackgroundColor.b, 1.f);
	
	int vertexStartingIndex = 0;
	int baseVertex = 0;
 	for (const Mesh* opaqueMesh : opaqueObjectsToBeRendered_)
	{
		opaqueMesh->Render();

		int facePointCount = opaqueMesh->GetFaceCount() * 3;
		glDrawElementsBaseVertex(GL_TRIANGLES, facePointCount, GL_UNSIGNED_INT, (void*)vertexStartingIndex, baseVertex);
		vertexStartingIndex += facePointCount * sizeof(Face::vertexIndices[0]);
		baseVertex += opaqueMesh->GetVertexCount();
	}

	for (const Mesh* maskedMesh : maskedObjectsToBeRendered_)
	{
		maskedMesh->Render();

		int facePointCount = maskedMesh->GetFaceCount() * 3;
		glDrawElementsBaseVertex(GL_TRIANGLES, facePointCount, GL_UNSIGNED_INT, (void*)vertexStartingIndex, baseVertex);
		vertexStartingIndex += facePointCount * sizeof(Face::vertexIndices[0]);
		baseVertex += maskedMesh->GetVertexCount();
	}

	glEnable(GL_BLEND);
	for (const Mesh* translucentMesh : translucentObjectsToBeRendered_)
	{
		translucentMesh->Render();

		int facePointCount = translucentMesh->GetFaceCount() * 3;
		glDrawElementsBaseVertex(GL_TRIANGLES, facePointCount, GL_UNSIGNED_INT, (void*)vertexStartingIndex, baseVertex);
		vertexStartingIndex += facePointCount * sizeof(Face::vertexIndices[0]);
		baseVertex += translucentMesh->GetVertexCount();
	}
	glDisable(GL_BLEND);
}

void Renderer::AddObjectToRenderer(Mesh* object)
{
	MaterialShadingModel materialShadingModel = object->GetMaterial()->GetShadingModel();
	switch (materialShadingModel)
	{
	case MaterialShadingModel::Opaque:
		opaqueObjectsToBeRendered_.push_back(object);
		break;
	case MaterialShadingModel::Masked:
		maskedObjectsToBeRendered_.push_back(object);
		break;
	case MaterialShadingModel::Translucent:
		translucentObjectsToBeRendered_.push_back(object);
		break;
	default:
		break;
	}
}
