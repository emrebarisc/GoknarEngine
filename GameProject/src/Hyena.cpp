#include "Hyena.h"

#include "Goknar/Model/2D/AnimatedSpriteMesh.h"
#include "Goknar/Components/AnimatedSpriteComponent.h"
#include "Goknar/Application.h"
#include "Goknar/Engine.h"
#include "Goknar/Managers/InputManager.h"
#include "Goknar/Material.h"
#include "Goknar/Math.h"
#include "Goknar/Renderer/Texture.h"
#include "Goknar/Scene.h"
#include "Goknar/Timer.h"

#include "Game.h"

#include "Deceased.h"

Hyena::Hyena() :
	ObjectBase(),
	velocity_(0.5f), 
	movementDirection_(Vector3::ZeroVector)
{
	SetTickable(true);

	Texture* texture = new Texture("./Content/Sprites/Hyena.png");
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
	attackAnimation->AddTextureCoordinate(Rect(Vector2(192.f, 96.f), Vector2(239.f, 143.f)));
	attackAnimation->AddTextureCoordinate(Rect(Vector2(240.f, 96.f), Vector2(287.f, 143.f)));
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

	hyenaSprite_ = new AnimatedSpriteComponent(this);
	hyenaSprite_->SetMesh(spriteMesh);
	hyenaSprite_->SetPivotPoint(Vector3(-0.1f, -0.1f, 0.f));
	hyenaSprite_->SetRelativeRotation(Vector3(0.f, 0.f, DEGREE_TO_RADIAN(180.f)));

	attackTimer_ = new Timer();
	attackTimer_->SetTicksPerSecond(2);
	attackTimer_->CallOnTick(std::bind(&Hyena::Attack, this));
}

Hyena::~Hyena()
{
}

void Hyena::BeginGame()
{
	hyenaSprite_->SetRelativePosition(Vector3::ZeroVector);
	SetWorldPosition(Vector3(3.f, 3.f, 0.f));
	hyenaSprite_->GetAnimatedSpriteMesh()->PlayAnimation("idle");

	//InputManager* inputManager = engine->GetInputManager();
	//if (inputManager)
	//{
	//	inputManager->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_PRESS, std::bind(&Hyena::WalkForward, this));
	//	inputManager->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_PRESS, std::bind(&Hyena::WalkBackward, this));
	//	inputManager->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_PRESS, std::bind(&Hyena::WalkLeftward, this));
	//	inputManager->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_PRESS, std::bind(&Hyena::WalkRightward, this));

	//	inputManager->AddKeyboardInputDelegate(KEY_MAP::S, INPUT_ACTION::G_RELEASE, std::bind(&Hyena::WalkForwardStopped, this));
	//	inputManager->AddKeyboardInputDelegate(KEY_MAP::W, INPUT_ACTION::G_RELEASE, std::bind(&Hyena::WalkBackwardStopped, this));
	//	inputManager->AddKeyboardInputDelegate(KEY_MAP::D, INPUT_ACTION::G_RELEASE, std::bind(&Hyena::WalkLeftwardStopped, this));
	//	inputManager->AddKeyboardInputDelegate(KEY_MAP::A, INPUT_ACTION::G_RELEASE, std::bind(&Hyena::WalkRightwardStopped, this));
	//}
}

void Hyena::Tick(float deltaTime)
{
	Game* game = dynamic_cast<Game*>(engine->GetApplication());
	if (!game)
	{
		return;
	}

	Deceased* deceasedCharacter = game->GetDeceased();

	Vector3 toCharacter = deceasedCharacter->GetWorldPosition() - GetWorldPosition();
	float toCharacterLength = toCharacter.Length();

	static float elapsedTime = 0.f;

	if (0.75f < toCharacterLength)
	{
		attackTimer_->Deactivate();
		movementDirection_ = toCharacter / toCharacterLength;

		if (0 < movementDirection_.Length())
		{
			hyenaSprite_->GetAnimatedSpriteMesh()->PlayAnimation("walk");
			SetWorldPosition(GetWorldPosition() + movementDirection_.GetNormalized() * velocity_ * deltaTime);
			float angle = RADIAN_TO_DEGREE(atan2(movementDirection_.x, movementDirection_.y));
			if (-45.f <= angle && angle <= 135.f)
			{
				SetWorldScaling(Vector3(-1.f, -1.f, 1.f));
			}
			else
			{
				SetWorldScaling(Vector3(1.f, 1.f, 1.f));
			}
		}
	}
	else
	{
		if (deceasedCharacter->GetIsDead())
		{
			hyenaSprite_->GetAnimatedSpriteMesh()->PlayAnimation("idle");
			attackTimer_->Deactivate();
		}
		else
		{
			if(!attackTimer_->GetIsActive()) Attack();
			attackTimer_->Activate();
		}
	}
}

void Hyena::Attack()
{
	Game* game = dynamic_cast<Game*>(engine->GetApplication());
	if (!game)
	{
		return;
	}
	Deceased* deceasedCharacter = game->GetDeceased();
	movementDirection_ = Vector3::ZeroVector;
	deceasedCharacter->Hurt(10.f);
	hyenaSprite_->GetAnimatedSpriteMesh()->PlayAnimation("attack");
}

void Hyena::WalkForward()
{
	movementDirection_ += Vector3::ForwardVector;
	DetermineAnimation();
}

void Hyena::WalkForwardStopped()
{
	movementDirection_ -= Vector3::ForwardVector;
	DetermineAnimation();
}

void Hyena::WalkBackward()
{
	movementDirection_ -= Vector3::ForwardVector;
	DetermineAnimation();
}

void Hyena::WalkBackwardStopped()
{
	movementDirection_ += Vector3::ForwardVector;
	DetermineAnimation();
}

void Hyena::WalkLeftward()
{
	movementDirection_ += Vector3::LeftVector;
	DetermineAnimation();
}

void Hyena::WalkLeftwardStopped()
{
	movementDirection_ -= Vector3::LeftVector;
	DetermineAnimation();
}

void Hyena::WalkRightward()
{
	movementDirection_ -= Vector3::LeftVector;
	DetermineAnimation();
}

void Hyena::WalkRightwardStopped()
{
	movementDirection_ += Vector3::LeftVector;
	DetermineAnimation();
}

void Hyena::DetermineAnimation()
{
	if (0.001f < movementDirection_.Length())
	{
		hyenaSprite_->GetAnimatedSpriteMesh()->PlayAnimation("walk");
	}
	else
	{
		hyenaSprite_->GetAnimatedSpriteMesh()->PlayAnimation("idle");
	}
}
