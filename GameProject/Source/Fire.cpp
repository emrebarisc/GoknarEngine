#include "Fire.h"

#include "Application.h"
#include "Engine.h"
#include "Scene.h"
#include "Lights/PointLight.h"
#include "Math/GoknarMath.h"

Fire::Fire() :
	ObjectBase()
{
	SetIsTickable(true);

	fireLight_ = new PointLight();
	fireLight_->SetPosition(Vector3{ 15.f, 0.f, 2.f });
	fireLight_->SetColor(Vector3{ 0.7605246901512146f, 0.09758714586496353f, 0.015996338799595833f });
	fireLight_->SetIntensity(fireIntensity_);
	fireLight_->SetRadius(25.f);
	fireLight_->SetIsShadowEnabled(false);
	fireLight_->SetLightMobility(LightMobility::Dynamic);
	engine->GetApplication()->GetMainScene()->AddPointLight(fireLight_);
}

void Fire::BeginGame()
{
}

void Fire::Tick(float deltaTime)
{
	float scaledTime = engine->GetElapsedTime() * 2.f;
	float sinValue = sin(scaledTime);
	float cosValue = cos(scaledTime);
	fireLight_->SetIntensity(fireIntensity_ * (0.9f + tan(cos(scaledTime)) * tan(cos(GoknarMath::Pow(scaledTime, 1.25f))) * 0.1f));
}
