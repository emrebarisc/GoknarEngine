#include "pch.h"

#include "Renderer.h"

#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/Log.h"
#include "Goknar/Material.h"
#include "Goknar/Model/Mesh.h"
#include "Goknar/Model/MeshInstance.h"

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

void Renderer::Init()
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	for (Mesh* mesh : meshes_)
	{
		totalVertexSize_ += (unsigned int)mesh->GetVerticesPointer()->size();
		totalFaceSize_ += (unsigned int)mesh->GetFacesPointer()->size();
	}

	SetBufferData();
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
	unsigned int baseVertex = 0;
	unsigned int vertexStartingIndex = 0;

	int vertexOffset = 0;
	int faceOffset = 0;
	for (Mesh* mesh : meshes_)
	{
		mesh->SetBaseVertex(baseVertex);
		mesh->SetVertexStartingIndex(vertexStartingIndex);

		const VertexArray* vertexArrayPtr = mesh->GetVerticesPointer();
		int vertexSizeInBytes = (int)vertexArrayPtr->size() * sizeof(vertexArrayPtr->at(0));
		glBufferSubData(GL_ARRAY_BUFFER, vertexOffset, vertexSizeInBytes, &vertexArrayPtr->at(0));

		const FaceArray* faceArrayPtr = mesh->GetFacesPointer();
		int faceSizeInBytes = (int)faceArrayPtr->size() * sizeof(faceArrayPtr->at(0));
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, faceOffset, faceSizeInBytes, &faceArrayPtr->at(0));

		vertexOffset += vertexSizeInBytes;
		faceOffset += faceSizeInBytes;

		baseVertex += mesh->GetVertexCount();
		vertexStartingIndex += mesh->GetFaceCount() * 3 * (int)sizeof(Face::vertexIndices[0]);
	}

	// Vertex position
	long long offset = 0;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (GEsizei)sizeOfVertexData, (void*)offset);
	glEnableVertexAttribArray(0);

	// Vertex normal
	offset += sizeof(VertexData::position);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (GEsizei)sizeOfVertexData, (void*)offset);
	glEnableVertexAttribArray(1);

	// Vertex color
	offset += sizeof(VertexData::normal);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (GEsizei)sizeOfVertexData, (void*)offset);

	// Vertex UV
	offset += sizeof(VertexData::color);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, (GEsizei)sizeOfVertexData, (void*)offset);
}

void Renderer::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	const Colorf& sceneBackgroundColor = engine->GetApplication()->GetMainScene()->GetBackgroundColor();
	glClearColor(sceneBackgroundColor.r, sceneBackgroundColor.g, sceneBackgroundColor.b, 1.f);
	
 	for (const MeshInstance* opaqueMeshInstance : opaqueMeshInstances_)
	{
		const Mesh* mesh = opaqueMeshInstance->GetMesh();
		opaqueMeshInstance->Render();

		int facePointCount = mesh->GetFaceCount() * 3;
		glDrawElementsBaseVertex(GL_TRIANGLES, facePointCount, GL_UNSIGNED_INT, (void*)mesh->GetVertexStartingIndex(), mesh->GetBaseVertex());
	}

	for (const MeshInstance* maskedMeshInstance : maskedMeshInstances_)
	{
		const Mesh* mesh = maskedMeshInstance->GetMesh();
		maskedMeshInstance->Render();

		int facePointCount = mesh->GetFaceCount() * 3;
		glDrawElementsBaseVertex(GL_TRIANGLES, facePointCount, GL_UNSIGNED_INT, (void*)mesh->GetVertexStartingIndex(), mesh->GetBaseVertex());
	}

	glEnable(GL_BLEND);
	for (const MeshInstance* translucentMeshInstance : translucentMeshInstances_)
	{
		const Mesh* mesh = translucentMeshInstance->GetMesh();
		translucentMeshInstance->Render();

		int facePointCount = mesh->GetFaceCount() * 3;
		glDrawElementsBaseVertex(GL_TRIANGLES, facePointCount, GL_UNSIGNED_INT, (void*)mesh->GetVertexStartingIndex(), mesh->GetBaseVertex());
	}
	glDisable(GL_BLEND);
}

void Renderer::AddMeshToRenderer(Mesh* mesh)
{
	meshes_.push_back(mesh);
}

void Renderer::AddMeshInstanceToRenderer(MeshInstance* meshInstance)
{
	MaterialShadingModel materialShadingModel = meshInstance->GetMesh()->GetMaterial()->GetShadingModel();
	switch (materialShadingModel)
	{
	case MaterialShadingModel::Opaque:
		opaqueMeshInstances_.push_back(meshInstance);
		break;
	case MaterialShadingModel::Masked:
		maskedMeshInstances_.push_back(meshInstance);
		break;
	case MaterialShadingModel::Translucent:
		translucentMeshInstances_.push_back(meshInstance);
		break;
	default:
		break;
	}
}
