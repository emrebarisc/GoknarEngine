#include "Deceased.h"

#include "Goknar/Model/2D/AnimatedSpriteMesh.h"
#include "Goknar/Components/AnimatedSpriteComponent.h"
#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/InputManager.h"
#include "Goknar/Material.h"
#include "Goknar/Math.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Scene.h"

Deceased::Deceased() :
	ObjectBase(),
	velocity_(1.f), 
	movementDirection_(Vector3::ZeroVector)
{
	SetTickable(true);

	Texture* texture = new Texture("./Content/Sprites/Deceased.png");
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
	spriteMesh->SetTicksPerSecond(12);

	AnimatedSpriteAnimation* idleAnimation = new AnimatedSpriteAnimation("idle");
	idleAnimation->AddTextureCoordinate(Rect(Vector2(0.f, 0.f), Vector2(47.f, 47.f)));
	idleAnimation->AddTextureCoordinate(Rect(Vector2(48.f, 0.f), Vector2(95.f, 47.f)));
	idleAnimation->AddTextureCoordinate(Rect(Vector2(96.f, 0.f), Vector2(143.f, 47.f)));
	idleAnimation->AddTextureCoordinate(Rect(Vector2(144.f, 0.f), Vector2(191.f, 47.f)));
	spriteMesh->AddAnimation(idleAnimation);

	AnimatedSpriteAnimation* walkAnimation = new AnimatedSpriteAnimation("walk");
	walkAnimation->AddTextureCoordinate(Rect(Vector2(0.f, 48.f), Vector2(47.f, 95.f)));
	walkAnimation->AddTextureCoordinate(Rect(Vector2(48.f, 48.f), Vector2(95.f, 95.f)));
	walkAnimation->AddTextureCoordinate(Rect(Vector2(96.f, 48.f), Vector2(143.f, 95.f)));
	walkAnimation->AddTextureCoordinate(Rect(Vector2(144.f, 48.f), Vector2(191.f, 95.f)));
	walkAnimation->AddTextureCoordinate(Rect(Vector2(192.f, 48.f), Vector2(239.f, 95.f)));
	walkAnimation->AddTextureCoordinate(Rect(Vector2(240.f, 48.f), Vector2(287.f, 95.f)));
	spriteMesh->AddAnimation(walkAnimation);

	AnimatedSpriteAnimation* attackAnimation = new AnimatedSpriteAnimation("attack");
	attackAnimation->AddTextureCoordinate(Rect(Vector2(0.f, 96.f), Vector2(47.f, 143.f)));
	attackAnimation->AddTextureCoordinate(Rect(Vector2(48.f, 96.f), Vector2(95.f, 143.f)));
	attackAnimation->AddTextureCoordinate(Rect(Vector2(96.f, 96.f), Vector2(143.f, 143.f)));
	attackAnimation->AddTextureCoordinate(Rect(Vector2(144.f, 96.f), Vector2(191.f, 143.f)));
	spriteMesh->AddAnimation(attackAnimation);

	AnimatedSpriteAnimation* hurtAnimation = new AnimatedSpriteAnimation("hurt");
	hurtAnimation->AddTextureCoordinate(Rect(Vector2(0.f, 144.f), Vector2(47.f, 191.f)));
	hurtAnimation->AddTextureCoordinate(Rect(Vector2(48.f, 144.f), Vector2(95.f, 191.f)));
	spriteMesh->AddAnimation(hurtAnimation);

	AnimatedSpriteAnimation* deathAnimation = new AnimatedSpriteAnimation("death");
	deathAnimation->AddTextureCoordinate(Rect(Vector2(0.f, 192.f), Vector2(47.f, 239.f)));
	deathAnimation->AddTextureCoordinate(Rect(Vector2(48.f, 192.f), Vector2(95.f, 239.f)));
	deathAnimation->AddTextureCoordinate(Rect(Vector2(96.f, 192.f), Vector2(143.f, 239.f)));
	deathAnimation->AddTextureCoordinate(Rect(Vector2(144.f, 192.f), Vector2(191.f, 239.f)));
	deathAnimation->AddTextureCoordinate(Rect(Vector2(192.f, 192.f), Vector2(239.f, 239.f)));
	deathAnimation->AddTextureCoordinate(Rect(Vector2(240.f, 192.f), Vector2(287.f, 239.f)));
	spriteMesh->AddAnimation(deathAnimation);

	spriteMesh->SetSize(1, 1);
	spriteMesh->SetPivotPoint(SPRITE_PIVOT_POINT::BOTTOM_MIDDLE);

	deceasedSprite_ = new AnimatedSpriteComponent(this);
	deceasedSprite_->SetMesh(spriteMesh);
	//deceasedSprite_->SetRelativeRotation(Vector3(0.f, 0.f, DEGREE_TO_RADIAN(180.f)));
}

Deceased::~Deceased()
{
}

void Deceased::BeginGame()
{
	deceasedSprite_->SetRelativePosition(Vector3::ZeroVector);
	deceasedSprite_->GetAnimatedSpriteMesh()->PlayAnimation("idle");

	InputManager* inputManager = engine->GetInputManager();
	if (inputManager)
	{
		inputManager->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_PRESS, std::bind(&Deceased::WalkForward, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_PRESS, std::bind(&Deceased::WalkBackward, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_PRESS, std::bind(&Deceased::WalkLeftward, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_PRESS, std::bind(&Deceased::WalkRightward, this));

		inputManager->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_RELEASE, std::bind(&Deceased::WalkForwardStopped, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_RELEASE, std::bind(&Deceased::WalkBackwardStopped, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_RELEASE, std::bind(&Deceased::WalkLeftwardStopped, this));
		inputManager->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_RELEASE, std::bind(&Deceased::WalkRightwardStopped, this));
	}
}

void Deceased::Tick(float deltaTime)
{
	if (0 < movementDirection_.Length())
	{
		SetWorldPosition(GetWorldPosition() + movementDirection_.GetNormalized() * velocity_ * deltaTime);
	}
}

void Deceased::WalkForward()
{
	movementDirection_ += Vector3::ForwardVector;
	DetermineAnimation();
}

void Deceased::WalkForwardStopped()
{
	movementDirection_ -= Vector3::ForwardVector;
	DetermineAnimation();
}

void Deceased::WalkBackward()
{
	movementDirection_ -= Vector3::ForwardVector;
	DetermineAnimation();
}

void Deceased::WalkBackwardStopped()
{
	movementDirection_ += Vector3::ForwardVector;
	DetermineAnimation();
}

void Deceased::WalkLeftward()
{
	movementDirection_ += Vector3::LeftVector;
	DetermineAnimation();
}

void Deceased::WalkLeftwardStopped()
{
	movementDirection_ -= Vector3::LeftVector;
	DetermineAnimation();
}

void Deceased::WalkRightward()
{
	movementDirection_ -= Vector3::LeftVector;
	DetermineAnimation();
}

void Deceased::WalkRightwardStopped()
{
	movementDirection_ += Vector3::LeftVector;
	DetermineAnimation();
}

void Deceased::DetermineAnimation()
{
	if (0.001f < movementDirection_.Length())
	{
		deceasedSprite_->GetAnimatedSpriteMesh()->PlayAnimation("walk");
	}
	else
	{
		deceasedSprite_->GetAnimatedSpriteMesh()->PlayAnimation("idle");
	}
}
