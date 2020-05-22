#include "Pipe.h"

#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Material.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Scene.h"
#include "Goknar/Model/2D/SpriteMesh.h"
#include "Goknar/Components/SpriteComponent.h"

Pipe::Pipe() :
	movementSpeed_(100.f)
{
	SetTickable(true);

	Texture* texture = new Texture("./Content/Sprites/ScaryMammal.png");
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
	shader->SetVertexShaderPath("./Content/Shaders/2DVertexShader.glsl");
	shader->SetFragmentShaderPath("./Content/Shaders/2DFragmentShader.glsl");
	shader->SetShaderType(ShaderType::SelfContained);
	shader->AddTexture(texture);
	scene->AddShader(shader);
	material->SetShader(shader);

	SpriteMesh* upperPipeMesh = new SpriteMesh(material);
	upperPipeMesh->SetTextureCoordinate(Rect(Vector2(0.f, 255.f), Vector2(25.f, 415.f))); 
	upperPipeSprite_ = new SpriteComponent(this);
	upperPipeSprite_->SetMesh(upperPipeMesh);
	upperPipeSprite_->SetRelativePosition(Vector3(0.f, 100.f, 0.f));

	SpriteMesh* lowerPipeMesh = new SpriteMesh(material);
	lowerPipeMesh->SetTextureCoordinate(Rect(Vector2(26.f, 256.f), Vector2(51.f, 415.f)));
	lowerPipeSprite_ = new SpriteComponent(this);
	lowerPipeSprite_->SetMesh(lowerPipeMesh);
	lowerPipeSprite_->SetRelativePosition(Vector3(0.f, -120.f, 0.f));
}

Pipe::~Pipe()
{

}

void Pipe::BeginGame()
{
	
}

void Pipe::Tick(float deltaTime)
{
	Vector3 newWorldPosition = GetWorldPosition() - Vector3(movementSpeed_ * deltaTime, 0.f, 0.f);
	if (newWorldPosition.x < -50.f)
	{
		newWorldPosition.x += 300.f;
		newWorldPosition.y = -rand() % 100;

	}
	SetWorldPosition(newWorldPosition);
}