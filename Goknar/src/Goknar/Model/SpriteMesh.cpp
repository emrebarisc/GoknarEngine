#include "pch.h"

#include "SpriteMesh.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Scene.h"

SpriteMesh::SpriteMesh() : texture_(nullptr)
{
	Scene* scene = engine->GetApplication()->GetMainScene();

	Material* material = new Material();
	material->SetBlendModel(MaterialBlendModel::Masked);
	material->SetShadingModel(MaterialShadingModel::TwoSided);
	material->SetDiffuseReflectance(Vector3(1.f));
	material->SetSpecularReflectance(Vector3(0.f));
	material->SetShadingModel(MaterialShadingModel::TwoSided);
	SetMaterial(material);
	scene->AddMaterial(material);

	AddVertexData(VertexData(Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(0.f, 0.f)));
	AddVertexData(VertexData(Vector3(1.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(1.f, 0.f)));
	AddVertexData(VertexData(Vector3(0.f, -1.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(0.f, 1.f)));
	AddVertexData(VertexData(Vector3(1.f, -1.f, 0.f), Vector3(0.f, 0.f, 1.f), Vector4::ZeroVector, Vector2(1.f, 1.f)));

	AddFace(Face(1, 0, 2));
	AddFace(Face(1, 2, 3));

	scene->AddMesh(this);
}

SpriteMesh::SpriteMesh(Texture* texture) : SpriteMesh()
{
	Scene* scene = engine->GetApplication()->GetMainScene();

	texture_ = texture;
	scene->AddTexture(texture);
	
	Shader* shader = new Shader();
	shader->SetVertexShaderPath("./Content/2DScene/Shaders/2DVertexShader.glsl");
	shader->SetFragmentShaderPath("./Content/2DScene/Shaders/2DFragmentShader.glsl");
	shader->SetShaderType(ShaderType::SelfContained);
	shader->AddTexture(texture_);
	scene->AddShader(shader);

	GetMaterial()->SetShader(shader);
}

void SpriteMesh::Init()
{
	Mesh::Init();
}
