#include "Bat.h"

#include "Goknar/Model/2D/AnimatedSpriteMesh.h"
#include "Goknar/Components/AnimatedSpriteComponent.h"
#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/InputManager.h"
#include "Goknar/Material.h"
#include "Goknar/Math.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Scene.h"

const float JUMP_SPEED = 250.f;
const float JUMP_END_SPEED = 666.667f;
const float FALL_SPEED = -250.f;

Bat::Bat() :
	ObjectBase(),
	verticalVelocity_(0)
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

	AnimatedSpriteMesh* spriteMesh = new AnimatedSpriteMesh(material);
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
	SetWorldPosition(Vector3(25.f, -128.f, 5.f));

	batSprite_->SetPivotPoint(Vector3(8.5f, -6.5f, 0.f));;
	batSprite_->GetAnimatedSpriteMesh()->PlayAnimation("fly");

	InputManager* inputManager = engine->GetInputManager();
	if (inputManager)
	{
		inputManager->AddKeyboardInputDelegate(KEY_MAP::SPACE, INPUT_ACTION::G_PRESS, std::bind(&Bat::Jump, this));
	}
}

void Bat::Tick(float deltaTime)
{
	if (FALL_SPEED < verticalVelocity_)
	{
		this->verticalVelocity_ -= JUMP_END_SPEED * deltaTime;
	}

	Vector3 worldPosition = GetWorldPosition();
	worldPosition.y += verticalVelocity_ * deltaTime;
	SetWorldPosition(worldPosition);

	batSprite_->SetRelativeRotation(Vector3(0.f, 0.f, DEGREE_TO_RADIAN(mathClamp(30.f + -30.f * (JUMP_SPEED - verticalVelocity_) / JUMP_SPEED, -30.f, 30.f))));
}

void Bat::Jump()
{
	if (verticalVelocity_ <= JUMP_SPEED) verticalVelocity_ = JUMP_SPEED;
}
