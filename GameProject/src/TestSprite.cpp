#include "TestSprite.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Components/DynamicMeshComponent.h"
#include "Goknar/Scene.h"
#include "Goknar/Model/2D/AnimatedSpriteMesh.h"
#include "Goknar/Renderer/Texture.h"

TestSprite::TestSprite() : ObjectBase()
{
	Texture* texture = new Texture("./Content/2DScene/Textures/ScaryMammal.png");
	texture->SetName("texture0");
	texture->SetTextureMinFilter(TextureMinFilter::NEAREST);
	texture->SetTextureMagFilter(TextureMagFilter::NEAREST);

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

	AnimatedSpriteMesh* spriteMesh = new AnimatedSpriteMesh(material);
	spriteMesh->AddTextureCoordinate(Rect(Vector2(52.f, 256.f), Vector2(69.f, 269.f)));
	spriteMesh->AddTextureCoordinate(Rect(Vector2(70.f, 256.f), Vector2(87.f, 269.f)));
	spriteMesh->AddTextureCoordinate(Rect(Vector2(88.f, 256.f), Vector2(105.f, 269.f)));
	spriteMesh->SetTicksPerSecond(12);

	spriteMeshComponent_ = new DynamicMeshComponent(this);
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
