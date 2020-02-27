#include "pch.h"

#include "Renderer.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Scene.h"
#include "Goknar/Log.h"
#include "Goknar/Material.h"
#include "Goknar/Model/DynamicMesh.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Model/DynamicMeshInstance.h"
#include "Goknar/Model/StaticMeshInstance.h"

#include "Goknar/IO/IOManager.h"
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
	glDisableVertexAttribArray(3);

	glDeleteBuffers(1, &vertexBufferId_);
	glDeleteBuffers(1, &indexBufferId_);

	for (StaticMeshInstance* opaqueStaticMeshInstance : opaqueStaticMeshInstances_)
	{
		delete opaqueStaticMeshInstance;
	}

	for (StaticMeshInstance* maskedStaticMeshInstance : maskedStaticMeshInstances_)
	{
		delete maskedStaticMeshInstance;
	}

	for (StaticMeshInstance* translucentStaticMeshInstance : translucentStaticMeshInstances_)
	{
		delete translucentStaticMeshInstance;
	}

	for (DynamicMeshInstance* opaqueDynamicMeshInstance : opaqueDynamicMeshInstances_)
	{
		delete opaqueDynamicMeshInstance;
	}

	for (DynamicMeshInstance* maskedDynamicMeshInstance : maskedDynamicMeshInstances_)
	{
		delete maskedDynamicMeshInstance;
	}

	for (DynamicMeshInstance* translucentDynamicMeshInstance : translucentDynamicMeshInstances_)
	{
		delete translucentDynamicMeshInstance;
	}
}

void Renderer::Init()
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	for (StaticMesh* mesh : staticMeshes_)
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
	for (Mesh* mesh : staticMeshes_)
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

	// Vertex color
	long long offset = 0;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, (GEsizei)sizeOfVertexData, (void*)offset);

	// Vertex position
	offset += sizeof(VertexData::color);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (GEsizei)sizeOfVertexData, (void*)offset);

	// Vertex normal
	offset += sizeof(VertexData::position);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (GEsizei)sizeOfVertexData, (void*)offset);

	// Vertex UV
	offset += sizeof(VertexData::normal);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, (GEsizei)sizeOfVertexData, (void*)offset);
}

void Renderer::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	const Colorf& sceneBackgroundColor = engine->GetApplication()->GetMainScene()->GetBackgroundColor();
	glClearColor(sceneBackgroundColor.r, sceneBackgroundColor.g, sceneBackgroundColor.b, 1.f);
	
 	for (const StaticMeshInstance* opaqueStaticMeshInstance : opaqueStaticMeshInstances_)
	{
		if (!opaqueStaticMeshInstance->GetIsRendered()) continue;

		const Mesh* mesh = opaqueStaticMeshInstance->GetMesh();
		opaqueStaticMeshInstance->Render();

		int facePointCount = mesh->GetFaceCount() * 3;
		glDrawElementsBaseVertex(GL_TRIANGLES, facePointCount, GL_UNSIGNED_INT, (void*)mesh->GetVertexStartingIndex(), mesh->GetBaseVertex());
	}

	for (const MeshInstance* maskedStaticMeshInstance : maskedStaticMeshInstances_)
	{
		if (!maskedStaticMeshInstance->GetIsRendered()) continue;

		const Mesh* mesh = maskedStaticMeshInstance->GetMesh();
		maskedStaticMeshInstance->Render();

		int facePointCount = mesh->GetFaceCount() * 3;
		glDrawElementsBaseVertex(GL_TRIANGLES, facePointCount, GL_UNSIGNED_INT, (void*)mesh->GetVertexStartingIndex(), mesh->GetBaseVertex());
	}

	glEnable(GL_BLEND);
	for (const MeshInstance* translucentStaticMeshInstance : translucentStaticMeshInstances_)
	{
		if (!translucentStaticMeshInstance->GetIsRendered()) continue;
		const Mesh* mesh = translucentStaticMeshInstance->GetMesh();
		translucentStaticMeshInstance->Render();

		int facePointCount = mesh->GetFaceCount() * 3;
		glDrawElementsBaseVertex(GL_TRIANGLES, facePointCount, GL_UNSIGNED_INT, (void*)mesh->GetVertexStartingIndex(), mesh->GetBaseVertex());
	}
	glDisable(GL_BLEND);
}

void Renderer::AddStaticMeshToRenderer(StaticMesh* staticMesh)
{
	staticMeshes_.push_back(staticMesh);
}

void Renderer::AddDynamicMeshToRenderer(DynamicMesh* object)
{
	dynamicMeshes_.push_back(object);
}

void Renderer::AddDynamicMeshInstance(DynamicMeshInstance* object)
{
}

void Renderer::RemoveDynamicMeshInstance(DynamicMeshInstance* object)
{
}

void Renderer::AddStaticMeshInstance(StaticMeshInstance* meshInstance)
{
	MaterialBlendModel materialShadingModel = meshInstance->GetMesh()->GetMaterial()->GetBlendModel();
	switch (materialShadingModel)
	{
	case MaterialBlendModel::Opaque:
		opaqueStaticMeshInstances_.push_back(meshInstance);
		break;
	case MaterialBlendModel::Masked:
		maskedStaticMeshInstances_.push_back(meshInstance);
		break;
	case MaterialBlendModel::Translucent:
		translucentStaticMeshInstances_.push_back(meshInstance);
		break;
	default:
		break;
	}
}

void Renderer::RemoveStaticMeshInstance(StaticMeshInstance* object)
{
	int meshInstanceCount = opaqueStaticMeshInstances_.size();
	for (int meshInstanceIndex = 0; meshInstanceIndex < meshInstanceCount; meshInstanceIndex++)
	{
		if (opaqueStaticMeshInstances_[meshInstanceIndex] == object)
		{
			opaqueStaticMeshInstances_.erase(opaqueStaticMeshInstances_.begin() + meshInstanceIndex);
			return;
		}
	}
	
	meshInstanceCount = maskedStaticMeshInstances_.size();
	for (int meshInstanceIndex = 0; meshInstanceIndex < meshInstanceCount; meshInstanceIndex++)
	{
		if (maskedStaticMeshInstances_[meshInstanceIndex] == object)
		{
			maskedStaticMeshInstances_.erase(maskedStaticMeshInstances_.begin() + meshInstanceIndex);
			return;
		}
	}
	
	meshInstanceCount = translucentStaticMeshInstances_.size();
	for (int meshInstanceIndex = 0; meshInstanceIndex < meshInstanceCount; meshInstanceIndex++)
	{
		if (translucentStaticMeshInstances_[meshInstanceIndex] == object)
		{
			translucentStaticMeshInstances_.erase(translucentStaticMeshInstances_.begin() + meshInstanceIndex);
			return;
		}
	}
}
