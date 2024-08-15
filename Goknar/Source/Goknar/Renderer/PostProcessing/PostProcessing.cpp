#include "pch.h"

#include "PostProcessing.h"

#include "Goknar/Engine.h"
#include "Goknar/Model/StaticMesh.h"
#include "Goknar/Renderer/Renderer.h"
#include "Goknar/Renderer/Shader.h"

PostProcessingEffect::PostProcessingEffect()
{
	postProcessingMesh_ = new StaticMesh();
	postProcessingMesh_->AddVertex(Vector3{ -1.f, -1.f, 0.f });
	postProcessingMesh_->AddVertex(Vector3{ 3.f, -1.f, 0.f });
	postProcessingMesh_->AddVertex(Vector3{ -1.f, 3.f, 0.f });
	postProcessingMesh_->AddFace(Face{ 0, 1, 2 });
}

PostProcessingEffect::~PostProcessingEffect()
{
}

void PostProcessingEffect::PreInit()
{
	postProcessingMesh_->PreInit();
}

void PostProcessingEffect::Init()
{
	postProcessingMesh_->Init();
}

void PostProcessingEffect::PostInit()
{
	postProcessingMesh_->PostInit();
}

void PostProcessingEffect::Render()
{
	shader_->Use();
	engine->GetRenderer()->RenderStaticMesh(postProcessingMesh_);
}
