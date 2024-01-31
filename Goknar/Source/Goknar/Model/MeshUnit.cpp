#include "pch.h"

#include "MeshUnit.h"

#include "Application.h"
#include "Camera.h"
#include "Engine.h"
#include "Scene.h"
#include "Materials/Material.h"
#include "Renderer/Shader.h"

#include "Managers/CameraManager.h"
#include "IO/IOManager.h"
#include "Renderer/ShaderBuilder.h"

MeshUnit::MeshUnit() :
	material_(nullptr), 
	vertexCount_(0), 
	faceCount_(0),
	baseVertex_(0),
	vertexStartingIndex_(0),
	isInitialized_(false),
	meshType_(MeshType::None)
{
	vertices_ = new VertexArray();
	faces_ = new FaceArray();
}

MeshUnit::~MeshUnit()
{
	if (vertices_)
	{
		delete vertices_;
	}

	if (faces_)
	{
		delete faces_;
	}
}

void MeshUnit::PreInit()
{
	aabb_.CalculateSize();

	vertexCount_ = (int)vertices_->size();
	faceCount_ = (int)faces_->size();

	if (material_)
	{
		material_->Build(this);
		material_->PreInit();
	}
}

void MeshUnit::Init()
{
	if (material_)
	{
		material_->Init();
	}
}

void MeshUnit::PostInit()
{
	if (material_)
	{
		material_->PostInit();
	}

	isInitialized_ = true;
}

void MeshUnit::ClearDataFromMemory()
{
	vertices_->clear();
	delete vertices_;
	vertices_ = nullptr;

	faces_->clear();
	delete faces_;
	faces_ = nullptr;
}
