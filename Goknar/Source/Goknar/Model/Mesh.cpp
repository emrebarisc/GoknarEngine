#include "pch.h"

#include "Mesh.h"

#include "Application.h"
#include "Camera.h"
#include "Engine.h"
#include "Material.h"
#include "Scene.h"
#include "Renderer/Shader.h"

#include "Managers/CameraManager.h"
#include "IO/IOManager.h"
#include "Renderer/ShaderBuilder.h"

Mesh::Mesh() :
	material_(0), 
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

Mesh::~Mesh()
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

void Mesh::Init()
{
	vertexCount_ = (int)vertices_->size();
	faceCount_ = (int)faces_->size();

	if (material_)
	{
		Shader* shader = material_->GetShader();
		if(!shader)
		{
			shader = new Shader();
			material_->SetShader(shader);
			engine->GetApplication()->GetMainScene()->AddShader(shader);
		}

		if (shader->GetShaderType() == ShaderType::Scene)
		{
			ShaderBuilder::GetInstance()->BuildShader(this);
		}

		shader->Init();
		material_->Init();
	}

	isInitialized_ = true;
}

void Mesh::ClearDataFromMemory()
{
	vertices_->clear();
	delete vertices_;
	vertices_ = nullptr;

	faces_->clear();
	delete faces_;
	faces_ = nullptr;
}
