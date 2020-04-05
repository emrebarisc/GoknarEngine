#include "Bat.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Components/AnimatedSpriteComponent.h"
#include "Goknar/Scene.h"
#include "Goknar/Model/2D/AnimatedSpriteMesh.h"
#include "Goknar/Renderer/Texture.h"

// TODO: DELETE
AnimatedSpriteMesh* spriteMesh;
Bat::Bat() : ObjectBase()
{
	Texture* texture = new Texture("./Content/ScaryMammal/Sprites/ScaryMammal.png");
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
	shader->SetVertexShaderPath("./Content/ScaryMammal/Shaders/2DVertexShader.glsl");
	shader->SetFragmentShaderPath("./Content/ScaryMammal/Shaders/2DFragmentShader.glsl");
	shader->SetShaderType(ShaderType::SelfContained);
	shader->AddTexture(texture);
	scene->AddShader(shader);
	material->SetShader(shader);

	spriteMesh = new AnimatedSpriteMesh(material);
	spriteMesh->SetSize(17.f, 13.f);

	AnimatedSpriteAnimation* flyAnimation = new AnimatedSpriteAnimation("fly");
	flyAnimation->AddTextureCoordinate(Rect(Vector2(52.f, 256.f), Vector2(69.f, 269.f)));
	flyAnimation->AddTextureCoordinate(Rect(Vector2(70.f, 256.f), Vector2(87.f, 269.f)));
	flyAnimation->AddTextureCoordinate(Rect(Vector2(88.f, 256.f), Vector2(105.f, 269.f)));

	spriteMesh->AddAnimation(flyAnimation);
	spriteMesh->SetTicksPerSecond(12);

	batSprite_ = new AnimatedSpriteComponent(this);
	batSprite_->SetMesh(spriteMesh);
}

Bat::~Bat()
{
}

void Bat::BeginGame()
{
	spriteMesh->PlayAnimation("fly");
	batSprite_->SetRelativePosition(Vector3(25.f, -128.f, 5.f));
}

void Bat::Tick(float deltaTime)
{
}
