#include "pch.h"

#include "MeshUnit.h"

#include "Application.h"
#include "Camera.h"
#include "Engine.h"
#include "Material.h"
#include "Scene.h"
#include "Renderer/Shader.h"

#include "Managers/CameraManager.h"
#include "IO/IOManager.h"
#include "Renderer/ShaderBuilder.h"

MeshUnit::MeshUnit() :
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

void MeshUnit::Init()
{
	vertexCount_ = (int)vertices_->size();
	faceCount_ = (int)faces_->size();

	if (material_)
	{
		material_->Build(this);
		material_->PreInit();
		material_->Init();
		material_->PostInit();
	}

	isInitialized_ = true;
}


//if (material_)
//{
//	Shader* shader = material_->GetShader(RenderPassType::Main);
//	if (!shader)
//	{
//		shader = new Shader();
//		material_->SetShader(shader);
//		engine->GetApplication()->GetMainScene()->AddShader(shader);
//	}
//
//	shader->PreInit();
//
//	if (shader->GetShaderType() == ShaderType::Scene)
//	{
//		ShaderBuilder::GetInstance()->BuildShader(this);
//	}
//
//	shader->Init();
//	material_->Init();
//}

void MeshUnit::ClearDataFromMemory()
{
	vertices_->clear();
	delete vertices_;
	vertices_ = nullptr;

	faces_->clear();
	delete faces_;
	faces_ = nullptr;
}
