#include "Environment.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Components/AnimatedSpriteComponent.h"
#include "Goknar/Components/SpriteComponent.h"
#include "Goknar/Scene.h"
#include "Goknar/Model/2D/AnimatedSpriteMesh.h"
#include "Goknar/Renderer/Texture.h"

#include "Pipe.h"

Environment::Environment() :
	ObjectBase()
{
	SetTickable(false);

	Texture* texture = new Texture("Sprites/ScaryMammal.png");
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
	shader->SetVertexShaderPath("Shaders/2DVertexShader.glsl");
	shader->SetFragmentShaderPath("Shaders/2DFragmentShader.glsl");
	shader->SetShaderType(ShaderType::SelfContained);
	shader->AddTexture(texture);
	scene->AddShader(shader);
	material->SetShader(shader);

	SpriteMesh* backgroundMesh = new SpriteMesh(material);
	backgroundMesh->SetTextureCoordinate(Rect(Vector2(0.f, 0.f), Vector2(143.f, 255.f)));
	backgroundMesh->SetSize(144.f, 256.f);
	background_ = new SpriteComponent(this);
	background_->SetMesh(backgroundMesh);
	
	SpriteMesh* floorMesh = new SpriteMesh(material);
	floorMesh->SetTextureCoordinate(Rect(Vector2(144.f, 0.f), Vector2(312.f, 54.f)));
	floorMesh->SetSize(169.f, 55.f);
	floor_ = new SpriteComponent(this);
	floor_->SetMesh(floorMesh);

	AnimatedSpriteMesh* firedSkullsMesh = new AnimatedSpriteMesh(material);
	firedSkullsMesh->SetTicksPerSecond(12);
	firedSkullsMesh->SetSize(126.f, 20.f);

	AnimatedSpriteAnimation* fireAnimation = new AnimatedSpriteAnimation("fire");
	fireAnimation->AddTextureCoordinate(Rect(Vector2(0.f, 419.f), Vector2(125.f, 439.f)));
	fireAnimation->AddTextureCoordinate(Rect(Vector2(126.f, 419.f), Vector2(251.f, 439.f)));
	fireAnimation->AddTextureCoordinate(Rect(Vector2(252.f, 419.f), Vector2(377.f, 439.f)));
	firedSkullsMesh->AddAnimation(fireAnimation);

	firedSkulls_ = new AnimatedSpriteComponent(this);
	firedSkulls_->SetMesh(firedSkullsMesh);

	pipe1_ = new Pipe();
	pipe2_ = new Pipe();
	pipe3_ = new Pipe();
}

Environment::~Environment()
{
}

void Environment::BeginGame()
{
	background_->SetRelativePosition(Vector3(0.f, 0.f, -0.3f));
	floor_->SetRelativePosition(Vector3(0.f, -246.f, 0.0f));
	firedSkulls_->SetRelativePosition(Vector3(9.f, -25.f, 0.1f));
	firedSkulls_->GetAnimatedSpriteMesh()->PlayAnimation("fire");

	pipe1_->SetWorldPosition(Vector3(100.f, 0.f, -0.2f));
	pipe2_->SetWorldPosition(Vector3(200.f, 0.f, -0.2f));
	pipe3_->SetWorldPosition(Vector3(300.f, 0.f, -0.2f));
}

void Environment::Tick(float deltaTime)
{
}