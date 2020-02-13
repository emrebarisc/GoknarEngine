#include "TestSprite.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Components/MeshComponent.h"
#include "Goknar/Scene.h"
#include "Goknar/Model/2D/SpriteMesh.h"
#include "Goknar/Renderer/Texture.h"

TestSprite::TestSprite()
{
	Texture* texture = new Texture("./Content/Textures/T_Airplane.png");
	texture->SetName("texture0");

	Scene* scene = engine->GetApplication()->GetMainScene();
	scene->AddTexture(texture);

	Material* material = new Material();
	material->SetBlendModel(MaterialBlendModel::Masked);
	material->SetShadingModel(MaterialShadingModel::TwoSided);
	material->SetDiffuseReflectance(Vector3(1.f));
	material->SetSpecularReflectance(Vector3(0.f));
	scene->AddMaterial(material);

	Shader* shader = new Shader();
	shader->SetVertexShaderPath("./Content/2DScene/Shaders/2DVertexShader.glsl");
	shader->SetFragmentShaderPath("./Content/2DScene/Shaders/2DFragmentShader.glsl");
	shader->SetShaderType(ShaderType::SelfContained);
	shader->AddTexture(texture);
	scene->AddShader(shader);
	material->SetShader(shader);

	SpriteMesh* spriteMesh = new SpriteMesh(material);
	spriteMeshComponent_ = new MeshComponent(this);
	spriteMeshComponent_->SetMesh(spriteMesh);
}

TestSprite::~TestSprite()
{
}

void TestSprite::BeginGame()
{
}

void TestSprite::Tick(float deltaTime)
{
}
